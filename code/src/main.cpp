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

inline static bool pulseFun(volatile uint16_t &counter, const uint16_t period, const bool state)
{
    ++counter;
    if (counter >= period)
    {
        counter = 0;
        return !state;
    }
    else return state;
}

static void tick()
{
    if (ledNew != led) digitalWrite(ONBOARD_LED_PIN, ledNew ? LOW : HIGH);
    if (lmotNew != lmot)
    {
        digitalWrite(LMOT_STEP_PIN, HIGH);
        digitalWrite(LMOT_STEP_PIN, LOW);
    }
    if (rmotNew != rmot)
    {
        digitalWrite(RMOT_STEP_PIN, HIGH);
        digitalWrite(RMOT_STEP_PIN, LOW);
    }
    led = ledNew;
    lmot = lmotNew;
    rmot = rmotNew;

    ledNew = pulseFun(ledc, 0x8000, led);
    // 32 -> 196; 512 ->
    if (lmotp != 0) lmotNew = pulseFun(lmotc, lmotp, lmot);
    if (rmotp != 0) rmotNew = pulseFun(rmotc, rmotp, rmot);
}

void setup()
{
    pinMode(ONBOARD_LED_PIN, OUTPUT);
    digitalWrite(ONBOARD_LED_PIN, HIGH);

    // CPU clock 10MHz: prescaler enabled, DIV2
    CCP = CCP_IOREG_gc;
    CLKCTRL.MCLKCTRLB = CLKCTRL_PDIV_2X_gc | CLKCTRL_PEN_bm;

    pinMode(PIN_PB3, INPUT);
    attachInterrupt(digitalPinToInterrupt(PIN_PB3), tick, RISING);

    pinMode(RMOT_STEP_PIN, OUTPUT);
    digitalWrite(RMOT_STEP_PIN, LOW);

    pinMode(RMOT_DIR_PIN, OUTPUT);
    digitalWrite(RMOT_DIR_PIN, LOW);

    pinMode(LMOT_STEP_PIN, OUTPUT);
    digitalWrite(LMOT_STEP_PIN, LOW);

    pinMode(LMOT_DIR_PIN, OUTPUT);
    digitalWrite(LMOT_DIR_PIN, LOW);
}

void loop()
{
    double t = millis();
    t /= 1000.0;
    double s, e;

    double t1 = t * 0.02;
    s = cos(t1) + 1.0;
    if (t1 > TAU) s = 2.0;
    e = exp(s);
    double p1 = 32 + round(e * 12.0);

    double t2 = t * 0.04;
    s = cos(t2) + 1.0;
    if (t1 > TAU) s = 2.0;
    e = exp(s);
    double p2 = 32 + round(e * 12.0);

    if (t1 > TAU + 0.02) p1 = p2 = 0.0;

    cli();
    lmotp = p1;
    rmotp = p2;
    sei();
}
