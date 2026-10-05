#pragma once

// Brightness Cap: upper limit of every Channel's PWM duty, in percent.
// Protects the power LED from overheating.
#define BRIGHTNESS_CAP_PERCENT 60

// Fade-in when a Lighting starts, fade-out when the switch is turned off.
#define FADE_IN_MS 2000UL
#define FADE_OUT_MS 1000UL

// Rocker switch must read the same for this long to count as changed.
#define SWITCH_DEBOUNCE_MS 20

// Keep-Alive: pulse the dummy load for ON_MS out of every PERIOD_MS.
#define KEEP_ALIVE_ON_MS 500UL
#define KEEP_ALIVE_PERIOD_MS 10000UL

// Breathing: one breath, and one full hue revolution.
#define BREATHING_PERIOD_MS 4000UL
#define BREATHING_HUE_PERIOD_MS 60000UL

// Hue Cycle: one full revolution of the colour wheel.
#define HUE_CYCLE_PERIOD_MS 30000UL

// RGB Chase: crossfade from one Channel to the next.
#define RGB_CHASE_STEP_MS 2000UL

// Candle: orange base colour and flicker timing.
#define CANDLE_GREEN 70        // green relative to red (perceptual, 0..255)
#define CANDLE_MIN_LEVEL 110   // dimmest flicker (perceptual, 0..255)
#define CANDLE_SLOW_MS 700UL   // slow sway segment length
#define CANDLE_FAST_MS 110UL   // fast flutter segment length
