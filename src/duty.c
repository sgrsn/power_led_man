#include "duty.h"

#include "config.h"
#include "tables.h"

#define CAP_SCALE ((uint32_t)DUTY_FULL * BRIGHTNESS_CAP_PERCENT / 100)

uint16_t duty_from_level(uint8_t level)
{
    uint16_t linear = pgm_read_word(&gamma_table[level]);
    return ((uint32_t)linear * CAP_SCALE) >> 16;
}
