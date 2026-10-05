#include <avr/eeprom.h>
#include <avr/io.h>
#include <stdbool.h>

#include "config.h"
#include "duty.h"
#include "lamp.h"
#include "pwm.h"

// Rocker switch between PB2 (pin 7) and GND: closed = on.
#define SWITCH_PIN PB2
// Keep-Alive FET gate on PB3 (pin 2), switching the 100 ohm dummy load.
#define KEEP_ALIVE_PIN PB3

// Pattern Rotation: the Pattern for the next Lighting.
static uint8_t EEMEM next_pattern;

static void io_init(void)
{
    PORTB |= _BV(SWITCH_PIN);   // pull-up
    DDRB |= _BV(KEEP_ALIVE_PIN);
}

static bool switch_closed(void)
{
    return !(PINB & _BV(SWITCH_PIN));
}

static void keep_alive(uint32_t uptime_ms)
{
    if (uptime_ms % KEEP_ALIVE_PERIOD_MS < KEEP_ALIVE_ON_MS)
        PORTB |= _BV(KEEP_ALIVE_PIN);
    else
        PORTB &= ~_BV(KEEP_ALIVE_PIN);
}

// Advances the rotation as soon as a Lighting starts, so quick off/on
// toggling skips to the following Pattern. BOD guards the EEPROM write.
static uint8_t take_next_pattern(void)
{
    uint8_t current = pattern_rotation_current(eeprom_read_byte(&next_pattern));
    eeprom_update_byte(&next_pattern, pattern_rotation_next(current));
    return current;
}

int main(void)
{
    io_init();
    pwm_init();

    Lamp lamp;
    lamp_init(&lamp);

    bool on = false;            // debounced switch state
    uint16_t unstable_ms = 0;   // how long the raw reading has disagreed
    uint32_t uptime_ms = 0;
    uint8_t last_tick = pwm_ticks();
    for (;;) {
        uint8_t now;
        while ((now = pwm_ticks()) == last_tick) {
        }
        uint16_t dt = (uint8_t)(now - last_tick) * PWM_TICK_MS;
        last_tick = now;
        uptime_ms += dt;

        keep_alive(uptime_ms);

        if (switch_closed() == on) {
            unstable_ms = 0;
        } else if ((unstable_ms += dt) >= SWITCH_DEBOUNCE_MS) {
            unstable_ms = 0;
            on = !on;
            if (on)
                lamp_switch_on(&lamp, take_next_pattern());
            else
                lamp_switch_off(&lamp);
        }

        Rgb c;
        lamp_render(&lamp, dt, &c);
        pwm_set(duty_from_level(c.r), duty_from_level(c.g), duty_from_level(c.b));
    }
}
