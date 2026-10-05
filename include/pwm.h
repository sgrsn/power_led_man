#pragma once

#include <stdint.h>

// Tick period of the time base.
#define PWM_TICK_MS 4

// Starts ~31 kHz PWM on R = PB4 (pin 3), G = PB0 (pin 5), B = PB1 (pin 6),
// all off, and the time base. Enables interrupts.
void pwm_init(void);

// Duties in DUTY_FULL units (see duty.h); the fraction below one PWM step
// is spread over successive PWM periods by dithering.
void pwm_set(uint16_t r, uint16_t g, uint16_t b);

// Free-running count of PWM_TICK_MS ticks.
uint8_t pwm_ticks(void);
