#pragma once

#include <stdint.h>

#ifdef __AVR__
#include <avr/pgmspace.h>
#else
#define PROGMEM
#define pgm_read_byte(p) (*(const uint8_t *)(p))
#define pgm_read_word(p) (*(const uint16_t *)(p))
#endif

extern const uint16_t gamma_table[256] PROGMEM;
extern const uint8_t wave_table[256] PROGMEM;
