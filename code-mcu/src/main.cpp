#include <Wire.h>
#include <math.h>

// clang-format off
#define ONBOARD_LED_PORT    PORTA
#define ONBOARD_LED_BM      PIN4_bm
#define LMOT_DIR_PORT       PORTA
#define LMOT_DIR_BM         PIN7_bm
#define LMOT_STEP_PORT      PORTB
#define LMOT_STEP_BM        PIN5_bm
#define RMOT_DIR_PORT       PORTA
#define RMOT_DIR_BM         PIN3_bm
#define RMOT_STEP_PORT      PORTA
#define RMOT_STEP_BM        PIN2_bm
#define I2C_ADDRESS         0x40
#define RECV_BUF_SZ         32
#define LARGE_NUM           0xffffffff
// clang-format on

// I2C variables
static volatile uint8_t recv_buf[RECV_BUF_SZ];
static volatile uint16_t recv_count = 0;

// Global tick counter
uint32_t ticks = 0;

// Left motor period
uint32_t lmotp = 7812;
// Right motor period
uint32_t rmotp = 420;
// Motor control variables
uint32_t lmot_next = LARGE_NUM;
uint32_t rmot_next = LARGE_NUM;
// LED control variable
uint32_t led_off = LARGE_NUM;

static void onWireReceive(int bytecount);
static void onWireRequest();
static void consumeBuffer();
static void driveSignals();

int main()
{
    ONBOARD_LED_PORT.DIRSET = ONBOARD_LED_BM;
    ONBOARD_LED_PORT.OUTSET = ONBOARD_LED_BM;

    LMOT_DIR_PORT.DIRSET = LMOT_DIR_BM;
    LMOT_STEP_PORT.DIRSET = LMOT_STEP_BM;
    RMOT_DIR_PORT.DIRSET = RMOT_DIR_BM;
    RMOT_STEP_PORT.DIRSET = RMOT_STEP_BM;

    // Full CPU frequency
    CCP = CCP_IOREG_gc;
    CLKCTRL.MCLKCTRLB = 0x00;

    // Timer counter, prescaler 256
    // => increments 78125 times per second
    TCA0.SINGLE.CTRLA = 0;
    TCA0.SINGLE.CTRLB = 0;
    TCA0.SINGLE.CTRLD = 0;
    TCA0.SINGLE.INTCTRL = 0;
    TCA0.SINGLE.CTRLA = 0b1101;

    // I2C target
    Wire.begin(I2C_ADDRESS);
    Wire.onReceive(onWireReceive);
    Wire.onRequest(onWireRequest);

    uint16_t cnt_last = TCA0.SINGLE.CNT;
    while (true)
    {
        // 78125 Hz counter
        uint16_t cnt_now = TCA0.SINGLE.CNT;
        if (cnt_now != cnt_last)
        {
            // Update tick counter
            uint16_t delta = cnt_now - cnt_last;
            cnt_last = cnt_now;
            ticks += delta;

            // Skipped a tick? Flash led from ~50 msec
            if (delta > 1)
            {
                ONBOARD_LED_PORT.OUTCLR = ONBOARD_LED_BM;
                led_off = ticks + 4000;
            }

            // Pulse motors/LED if needed
            driveSignals();
        }

        // I2C commands
        cli();
        uint16_t count = recv_count;
        if (count != 0) consumeBuffer();
        sei();
    }

    return 0;
}

static void onWireReceive(int byteCount)
{
    while (byteCount > 0)
    {
        int dd = Wire.read();
        if (dd == -1) break;
        if (recv_count == RECV_BUF_SZ) continue;
        recv_buf[recv_count++] = (uint8_t)dd;
    }
}

static void onWireRequest()
{
}

static inline __attribute__((always_inline)) void consumeBuffer()
{
    if (recv_count == 0) return;
    const uint8_t cmd = recv_buf[0];
    uint8_t consumed = 0;
    if (cmd == 0xf0) // light off
    {
        ONBOARD_LED_PORT.OUTSET = ONBOARD_LED_BM;
        consumed = 1;
    }
    else if (cmd == 0xf1) // light on
    {
        ONBOARD_LED_PORT.OUTCLR = ONBOARD_LED_BM;
        consumed = 1;
    }
    else if (cmd == 0x10 && recv_count >= 5) // left motor period
    {
        consumed = 5;
        lmotp = 0;
        for (uint8_t i = 1; i < 5; ++i)
        {
            lmotp <<= 8;
            lmotp += recv_buf[i];
        }
        if (lmotp == 0) lmot_next = LARGE_NUM;
        else if (lmot_next == LARGE_NUM) lmot_next = ticks;
        else
        {
            while (lmot_next > ticks + lmotp)
                lmot_next -= lmotp;
        }
    }
    // Shift buffer left
    for (uint8_t i = 0; i + consumed < recv_count; ++i)
    {
        recv_buf[i] = recv_buf[i + consumed];
    }
    recv_count -= consumed;
}

static inline __attribute__((always_inline)) void driveSignals()
{
    if (ticks >= lmot_next)
    {
        LMOT_STEP_PORT.OUTSET = LMOT_STEP_BM;
        lmot_next += lmotp;
        LMOT_STEP_PORT.OUTCLR = LMOT_STEP_BM;
    }
    if (ticks >= rmot_next)
    {
        RMOT_STEP_PORT.OUTSET = RMOT_STEP_BM;
        rmot_next += rmotp;
        RMOT_STEP_PORT.OUTCLR = RMOT_STEP_BM;
    }
    if (ticks >= led_off)
    {
        ONBOARD_LED_PORT.OUTSET = ONBOARD_LED_BM;
        led_off = LARGE_NUM;
    }
}
