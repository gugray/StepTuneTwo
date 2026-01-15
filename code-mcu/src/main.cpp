#include <Arduino.h>
#include <Wire.h>
#include <math.h>

// clang-format off
#define ONBOARD_LED_PIN     PIN_PA4
#define RMOT_DIR_PIN        PIN_PA3
#define RMOT_STEP_PIN       PIN_PA2
#define LMOT_DIR_PIN        PIN_PA7
#define LMOT_STEP_PIN       PIN_PB5
#define I2C_ADDRESS         0x40
#define RECV_BUF_SZ         32
// clang-format on

static volatile uint8_t recvBuf[RECV_BUF_SZ];
static volatile uint16_t recvCount = 0;

static void onWireReceive(int bytecount);
static void onWireRequest();
static void consumeBuffer();

int main()
{
    init();

    pinMode(ONBOARD_LED_PIN, OUTPUT);
    digitalWrite(ONBOARD_LED_PIN, HIGH);

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
    uint32_t ticks = 0;
    while (true)
    {
        // 78125 Hz counter
        uint16_t cnt_now = TCA0.SINGLE.CNT;
        if (cnt_now != cnt_last)
        {
            uint16_t delta = cnt_now - cnt_last;
            cnt_last = cnt_now;
            ticks += delta;
        }

        // I2C commands
        cli();
        uint16_t count = recvCount;
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
        if (recvCount == RECV_BUF_SZ) continue;
        recvBuf[recvCount++] = (uint8_t)dd;
    }
}

static void onWireRequest()
{
}

void consumeBuffer()
{
    const uint8_t cmd = recvBuf[0];
    recvCount = 0;
    if (cmd == 0xf0) // light off
    {
        digitalWrite(ONBOARD_LED_PIN, HIGH);
    }
    else if (cmd == 0xf1) // light on
    {
        digitalWrite(ONBOARD_LED_PIN, LOW);
    }
}
