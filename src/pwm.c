#include "pwm.h"

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/atomic.h>

// 8 MHz / 256 = 31250 PWM periods per second; 125 of them make 4 ms.
#define PERIODS_PER_TICK 125

// Integer and 4-bit fractional part of each Channel's duty.
static volatile uint8_t whole_r, whole_g, whole_b;
static volatile uint8_t frac_r, frac_g, frac_b;
static volatile uint8_t ticks;

void pwm_init(void)
{
    // G = OC0A (PB0), B = OC0B (PB1): Timer0 fast PWM, no prescaler,
    // inverting so that OCR0x = 255 is a clean, spike-free off.
    OCR0A = 255;
    OCR0B = 255;
    TCCR0A = _BV(COM0A1) | _BV(COM0A0) | _BV(COM0B1) | _BV(COM0B0) |
             _BV(WGM01) | _BV(WGM00);
    TCCR0B = _BV(CS00);

    // R = OC1B (PB4): Timer1 PWM, TOP = OCR1C, no prescaler.
    // OCR1B = 0 holds the output low.
    OCR1C = 255;
    OCR1B = 0;
    GTCCR = _BV(PWM1B) | _BV(COM1B1);
    TCCR1 = _BV(CS10);

    DDRB |= _BV(PB0) | _BV(PB1) | _BV(PB4);

    TIMSK |= _BV(TOIE0);
    sei();
}

void pwm_set(uint16_t r, uint16_t g, uint16_t b)
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        whole_r = r >> 4;
        frac_r = r & 0x0F;
        whole_g = g >> 4;
        frac_g = g & 0x0F;
        whole_b = b >> 4;
        frac_b = b & 0x0F;
    }
}

uint8_t pwm_ticks(void)
{
    return ticks;
}

// Once per PWM period: first-order dithering, so that over 16 periods the
// average duty carries the 4 fractional bits (below the flicker threshold
// at ~2 kHz). Also drives the time base.
ISR(TIMER0_OVF_vect)
{
    static uint8_t acc_r, acc_g, acc_b, periods;
    uint8_t d;

    acc_r += frac_r;
    d = whole_r;
    if (acc_r >= 16) {
        acc_r -= 16;
        d++;
    }
    OCR1B = d;

    acc_g += frac_g;
    d = whole_g;
    if (acc_g >= 16) {
        acc_g -= 16;
        d++;
    }
    OCR0A = 255 - d;

    acc_b += frac_b;
    d = whole_b;
    if (acc_b >= 16) {
        acc_b -= 16;
        d++;
    }
    OCR0B = 255 - d;

    if (++periods == PERIODS_PER_TICK) {
        periods = 0;
        ticks++;
    }
}
