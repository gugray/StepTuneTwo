#include <Arduino.h>
#include <Wire.h>
#include <math.h>

#include "magic.h"

#define PI 3.141592653589793
#define TAU 6.28318548

#define OSC32K_ENABLE_bm (1 << 0)
#define OSC32K_RUNSTDBY_bm (1 << 6)

volatile uint16_t ledc = 0;
volatile bool led = false, ledNew = false;

volatile uint16_t rmotc = 0;
volatile uint16_t rmotp = 0;
volatile bool rmot = false;
volatile bool rmotNew = false;
volatile uint16_t lmotc = 0;
volatile uint16_t lmotp = 0;
volatile bool lmot = false;
volatile bool lmotNew = false;

static inline __attribute__((always_inline)) bool pulseFun(
    volatile uint16_t &counter,
    const uint16_t period,
    const bool state)
{
    ++counter;
    if (counter >= period)
    {
        counter = 0;
        return !state;
    }
    else return state;
}

static inline __attribute__((always_inline)) void tick()
{
    if (ledNew != led)
    {
        if (ledNew) VPORTA.OUT &= ~PIN4_bm;
        else VPORTA.OUT |= PIN4_bm;
        // digitalWrite(ONBOARD_LED_PIN, ledNew ? LOW : HIGH);
    }
    if (lmotNew != lmot)
    {
        VPORTB.OUT |= PIN5_bm;
        VPORTB.OUT &= ~PIN5_bm;
    }
    if (rmotNew != rmot)
    {
        VPORTA.OUT |= PIN2_bm;
        VPORTA.OUT &= ~PIN2_bm;
    }
    led = ledNew;
    lmot = lmotNew;
    rmot = rmotNew;

    ledNew = pulseFun(ledc, 40000, led);
    // 32 -> 196; 512 ->
    if (lmotp != 0) lmotNew = pulseFun(lmotc, lmotp, lmot);
    if (rmotp != 0) rmotNew = pulseFun(rmotc, rmotp, rmot);
}

void setup()
{
    pinMode(ONBOARD_LED_PIN, OUTPUT);
    digitalWrite(ONBOARD_LED_PIN, HIGH);

    // CPU clock 10MHz: prescaler enabled, DIV2
    // CCP = CCP_IOREG_gc;
    // CLKCTRL.MCLKCTRLB = CLKCTRL_PDIV_2X_gc | CLKCTRL_PEN_bm;

    TCB1.CTRLA = 0;
    TCB1.CCMP = 249;
    TCB1.CTRLB = TCB_CNTMODE_INT_gc;
    TCB1.INTFLAGS = TCB_CAPT_bm;
    TCB1.INTCTRL = TCB_CAPT_bm;
    TCB1.CTRLA = TCB_ENABLE_bm;

    // pinMode(PIN_PB3, INPUT);
    // attachInterrupt(digitalPinToInterrupt(PIN_PB3), tick, RISING);

    pinMode(RMOT_STEP_PIN, OUTPUT);
    digitalWrite(RMOT_STEP_PIN, LOW);

    pinMode(RMOT_DIR_PIN, OUTPUT);
    digitalWrite(RMOT_DIR_PIN, LOW);

    pinMode(LMOT_STEP_PIN, OUTPUT);
    digitalWrite(LMOT_STEP_PIN, LOW);

    pinMode(LMOT_DIR_PIN, OUTPUT);
    digitalWrite(LMOT_DIR_PIN, LOW);
}

ISR(TCB1_INT_vect)
{
    tick();
    TCB1.INTFLAGS = TCB_CAPT_bm;
}

void loop()
{
    // delay(1);
    // return;

    double t = millis();
    double pw = t / 30000;

    if (pw > 5) pw = 10 - pw;
    if (pw <= 0)
    {
        cli();
        lmotp = rmotp = 0;
        sei();
        delay(100);
        return;
    }

    double lPw = pw;
    double rPw = pw;

    // PW goes 0 -> 5 -> 0
    lPw = sin(PI * pw / 5) * 5;  // ^ - v
    rPw = sin(PI * pw / 10) * 5; // ^   -

    // 13.75 Hz -> 440 Hz: 5 octaves: OK
    // From about 880: sound, not rot
    // Let's do this in 60 sec

    // t/60000: 0 -> 1 over 60 sec
    // t/10000: 0 -> 6 over 60 sec
    double lFreq = 13.75 * pow(2, lPw);
    double lPer = round(80000 / lFreq);
    double rFreq = 13.75 * pow(2, rPw * 0.88333);
    double rPer = round(80000 / rFreq);

    cli();
    lmotp = lPer;
    rmotp = rPer;
    sei();

    delay(1);

    // // t /= 1000.0;
    // t /= 200.0;
    // double s, e;

    // double t1 = t * 0.02;
    // s = cos(t1) + 1.0;
    // if (t1 > TAU) s = 2.0;
    // e = exp(s);
    // double p1 = 64 + round(e * 32.0); // <=

    // double t2 = t * 0.04;
    // s = cos(t2) + 1.0;
    // if (t1 > TAU) s = 2.0;
    // e = exp(s);
    // double p2 = 64 + round(e * 32.0);

    // if (t1 > TAU + 0.02) p1 = p2 = 0.0;

    // cli();
    // lmotp = p1;
    // rmotp = p2;
    // sei();
}
