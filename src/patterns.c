#include "patterns.h"

#include "config.h"
#include "tables.h"

// Raised cosine at a 16-bit phase: 0 at phase 0, 255 at phase 0x8000.
static uint8_t wave(uint16_t phase)
{
    uint8_t i = phase >> 8;
    uint8_t f = phase & 0xFF;
    int16_t a = pgm_read_byte(&wave_table[i]);
    int16_t b = pgm_read_byte(&wave_table[(uint8_t)(i + 1)]);
    return a + (((b - a) * f) >> 8);
}

// S-curve from 0 to 255 as x goes from 0 to 255.
uint8_t pattern_ease(uint8_t x)
{
    return wave((uint16_t)x << 7);
}

// Position within the current period as a 16-bit phase.
// Wraps once after ~49 days, when t_ms itself overflows.
static uint16_t phase_of(uint32_t t_ms, uint32_t period_ms)
{
    return ((t_ms % period_ms) << 16) / period_ms;
}

static uint8_t scale(uint8_t v, uint8_t s)
{
    return ((uint16_t)v * s + 255) >> 8;
}

static void scale_rgb(Rgb *c, uint8_t s)
{
    c->r = scale(c->r, s);
    c->g = scale(c->g, s);
    c->b = scale(c->b, s);
}

// Fully saturated colour on the hue wheel, hue 0..65535.
static void hue_rgb(uint16_t hue, Rgb *out)
{
    uint16_t h6 = ((uint32_t)hue * 6) >> 8;  // 0..1535
    uint8_t f = h6 & 0xFF;
    switch (h6 >> 8) {
    case 0:  *out = (Rgb){255, f, 0}; break;
    case 1:  *out = (Rgb){255 - f, 255, 0}; break;
    case 2:  *out = (Rgb){0, 255, f}; break;
    case 3:  *out = (Rgb){0, 255 - f, 255}; break;
    case 4:  *out = (Rgb){f, 0, 255}; break;
    default: *out = (Rgb){255, 0, 255 - f}; break;
    }
}

static uint8_t random8(PatternState *s)
{
    uint16_t x = s->rng;
    x ^= x << 7;
    x ^= x >> 9;
    x ^= x << 8;
    s->rng = x;
    return x >> 8;
}

// Random values joined by S-curves, so the flicker never jumps.
static uint8_t noise(PatternState *s, uint8_t *from, uint8_t *to,
                     uint32_t *start, uint32_t seg_ms, uint32_t t_ms)
{
    while (t_ms - *start >= seg_ms) {
        *start += seg_ms;
        *from = *to;
        *to = random8(s);
    }
    uint8_t x = ((t_ms - *start) << 8) / seg_ms;
    int16_t d = (int16_t)*to - *from;
    return *from + (d * pattern_ease(x)) / 255;
}

static void breathing(uint32_t t, Rgb *out)
{
    hue_rgb(phase_of(t, BREATHING_HUE_PERIOD_MS), out);
    scale_rgb(out, wave(phase_of(t, BREATHING_PERIOD_MS)));
}

static void hue_cycle(uint32_t t, Rgb *out)
{
    hue_rgb(phase_of(t, HUE_CYCLE_PERIOD_MS), out);
}

static void candle(PatternState *s, uint32_t t, Rgb *out)
{
    uint8_t slow = noise(s, &s->slow_from, &s->slow_to, &s->slow_start,
                         CANDLE_SLOW_MS, t);
    uint8_t fast = noise(s, &s->fast_from, &s->fast_to, &s->fast_start,
                         CANDLE_FAST_MS, t);
    uint8_t mix = ((uint16_t)slow * 3 + fast) >> 2;
    uint8_t level = CANDLE_MIN_LEVEL + scale(255 - CANDLE_MIN_LEVEL, mix);
    *out = (Rgb){255, CANDLE_GREEN, 0};
    scale_rgb(out, level);
}

static void rgb_chase(uint32_t t, Rgb *out)
{
    uint32_t step = t / RGB_CHASE_STEP_MS;
    uint8_t x = ((t % RGB_CHASE_STEP_MS) << 8) / RGB_CHASE_STEP_MS;
    uint8_t in = pattern_ease(x);
    uint8_t ch[3] = {0, 0, 0};
    uint8_t k = step % 3;
    ch[k] = 255 - in;
    ch[(k + 1) % 3] = in;
    *out = (Rgb){ch[0], ch[1], ch[2]};
}

uint8_t pattern_rotation_current(uint8_t remembered)
{
    return remembered < PATTERN_COUNT ? remembered : 0;
}

uint8_t pattern_rotation_next(uint8_t current)
{
    return (current + 1) % PATTERN_COUNT;
}

void pattern_start(PatternState *s, uint8_t id)
{
    *s = (PatternState){.id = id, .rng = 0xACE1};
    s->slow_to = random8(s);
    s->fast_to = random8(s);
}

void pattern_render(PatternState *s, uint32_t t_ms, Rgb *out)
{
    switch (s->id) {
    case PATTERN_BREATHING: breathing(t_ms, out); break;
    case PATTERN_HUE_CYCLE: hue_cycle(t_ms, out); break;
    case PATTERN_CANDLE:    candle(s, t_ms, out); break;
    default:                rgb_chase(t_ms, out); break;
    }
    if (t_ms < FADE_IN_MS)
        scale_rgb(out, pattern_ease((t_ms << 8) / FADE_IN_MS));
}
