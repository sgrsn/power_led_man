#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "patterns.h"

// Combines the running Pattern with the fade-out of whatever was lit when the
// switch was last turned off, so every switch change is Smooth -- even rapid
// on/off/on toggling.
typedef struct {
    bool lit;
    PatternState pattern;
    uint32_t pattern_ms;
    // Frozen colour at the last switch-off, fading to black.
    Rgb fading;
    uint32_t fading_ms;
} Lamp;

void lamp_init(Lamp *l);

// Starts a Lighting running the given Pattern, fading in from black.
void lamp_switch_on(Lamp *l, uint8_t pattern_id);

// Ends the Lighting with a fade-out.
void lamp_switch_off(Lamp *l);

// Advances time by dt_ms and returns what to show now.
void lamp_render(Lamp *l, uint32_t dt_ms, Rgb *out);
