#pragma once

#include <stdint.h>

// Perceptual brightness of each Channel, 0 (off) .. 255 (full).
typedef struct {
    uint8_t r, g, b;
} Rgb;

enum {
    PATTERN_BREATHING,
    PATTERN_HUE_CYCLE,
    PATTERN_CANDLE,
    PATTERN_RGB_CHASE,
    PATTERN_COUNT
};

typedef struct {
    uint8_t id;
    // Candle flicker: two layers of smoothed random noise.
    uint16_t rng;
    uint8_t slow_from, slow_to, fast_from, fast_to;
    uint32_t slow_start, fast_start;
} PatternState;

// Pattern Rotation: the Pattern to run this Lighting, given the value
// remembered from the previous one (any out-of-range value picks the first).
uint8_t pattern_rotation_current(uint8_t remembered);
uint8_t pattern_rotation_next(uint8_t current);

// S-curve from 0 to 255 as x goes from 0 to 255.
uint8_t pattern_ease(uint8_t x);

void pattern_start(PatternState *s, uint8_t id);

// Brightness at t_ms since the Lighting began, including the fade-in.
// t_ms must never go backwards between calls.
void pattern_render(PatternState *s, uint32_t t_ms, Rgb *out);
