#pragma once

#include <stdint.h>

// PWM duty with 4 extra bits of dithering: 4096 == 100 %.
#define DUTY_FULL 4096U

// Perceptual level (0..255) -> gamma-corrected duty, limited by the
// Brightness Cap. 0 always maps to exactly 0 (fully off).
uint16_t duty_from_level(uint8_t level);
