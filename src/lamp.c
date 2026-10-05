#include "lamp.h"

#include "config.h"

// Fade-out factor: 255 at switch-off, easing down to 0 after FADE_OUT_MS.
static uint8_t fade_out(uint32_t ms)
{
    if (ms >= FADE_OUT_MS)
        return 0;
    return 255 - pattern_ease((ms << 8) / FADE_OUT_MS);
}

static uint8_t add(uint8_t a, uint8_t b)
{
    uint16_t s = (uint16_t)a + b;
    return s > 255 ? 255 : s;
}

static void current(Lamp *l, Rgb *out)
{
    uint8_t f = fade_out(l->fading_ms);
    *out = (Rgb){
        ((uint16_t)l->fading.r * f + 255) >> 8,
        ((uint16_t)l->fading.g * f + 255) >> 8,
        ((uint16_t)l->fading.b * f + 255) >> 8,
    };
    if (l->lit) {
        Rgb p;
        pattern_render(&l->pattern, l->pattern_ms, &p);
        out->r = add(out->r, p.r);
        out->g = add(out->g, p.g);
        out->b = add(out->b, p.b);
    }
}

void lamp_init(Lamp *l)
{
    *l = (Lamp){.fading_ms = FADE_OUT_MS};
}

void lamp_switch_on(Lamp *l, uint8_t pattern_id)
{
    pattern_start(&l->pattern, pattern_id);
    l->pattern_ms = 0;
    l->lit = true;
}

void lamp_switch_off(Lamp *l)
{
    if (!l->lit)
        return;
    current(l, &l->fading);
    l->fading_ms = 0;
    l->lit = false;
}

void lamp_render(Lamp *l, uint32_t dt_ms, Rgb *out)
{
    if (l->fading_ms < FADE_OUT_MS)
        l->fading_ms += dt_ms;
    if (l->lit)
        l->pattern_ms += dt_ms;
    current(l, out);
}
