#include <stdio.h>
#include <stdlib.h>
#include <unity.h>

#include "config.h"
#include "duty.h"
#include "patterns.h"

#define FRAME_MS 4
#define RUN_MS (3UL * 60 * 1000)

// Largest change of any Channel between consecutive 4 ms frames that still
// looks Smooth. 255 steps over ~0.5 s would be 2 per frame.
#define MAX_STEP 3

void setUp(void) {}
void tearDown(void) {}

static int step(uint8_t a, uint8_t b)
{
    return abs((int)a - (int)b);
}

static void check_smooth(uint8_t id)
{
    PatternState s;
    pattern_start(&s, id);
    Rgb prev;
    pattern_render(&s, 0, &prev);
    TEST_ASSERT_EQUAL_UINT8(0, prev.r);
    TEST_ASSERT_EQUAL_UINT8(0, prev.g);
    TEST_ASSERT_EQUAL_UINT8(0, prev.b);

    for (uint32_t t = FRAME_MS; t < RUN_MS; t += FRAME_MS) {
        Rgb c;
        pattern_render(&s, t, &c);
        int d = step(c.r, prev.r);
        if (step(c.g, prev.g) > d) d = step(c.g, prev.g);
        if (step(c.b, prev.b) > d) d = step(c.b, prev.b);
        if (d > MAX_STEP) {
            char msg[64];
            snprintf(msg, sizeof msg, "jump of %d at t=%lu ms", d, (unsigned long)t);
            TEST_FAIL_MESSAGE(msg);
        }
        prev = c;
    }
}

static void test_breathing_is_smooth(void) { check_smooth(PATTERN_BREATHING); }
static void test_hue_cycle_is_smooth(void) { check_smooth(PATTERN_HUE_CYCLE); }
static void test_candle_is_smooth(void) { check_smooth(PATTERN_CANDLE); }
static void test_rgb_chase_is_smooth(void) { check_smooth(PATTERN_RGB_CHASE); }

static void test_candle_keeps_flickering(void)
{
    PatternState s;
    pattern_start(&s, PATTERN_CANDLE);
    uint8_t lo = 255, hi = 0;
    for (uint32_t t = FADE_IN_MS; t < RUN_MS; t += FRAME_MS) {
        Rgb c;
        pattern_render(&s, t, &c);
        if (c.r < lo) lo = c.r;
        if (c.r > hi) hi = c.r;
        TEST_ASSERT_EQUAL_UINT8(0, c.b);
    }
    TEST_ASSERT_GREATER_OR_EQUAL(CANDLE_MIN_LEVEL, lo);
    TEST_ASSERT_GREATER_THAN(60, hi - lo);
}

static void test_rotation_visits_every_pattern_in_order(void)
{
    uint8_t p = pattern_rotation_current(0xFF);  // erased EEPROM
    TEST_ASSERT_EQUAL_UINT8(PATTERN_BREATHING, p);
    for (uint8_t i = 1; i <= PATTERN_COUNT; i++) {
        p = pattern_rotation_current(pattern_rotation_next(p));
        TEST_ASSERT_EQUAL_UINT8(i % PATTERN_COUNT, p);
    }
}

static void test_duty_respects_brightness_cap(void)
{
    TEST_ASSERT_EQUAL_UINT16(0, duty_from_level(0));
    uint16_t cap = DUTY_FULL * BRIGHTNESS_CAP_PERCENT / 100;
    TEST_ASSERT_LESS_OR_EQUAL(cap, duty_from_level(255));
    TEST_ASSERT_GREATER_OR_EQUAL(cap - 1, duty_from_level(255));
    for (int l = 1; l < 256; l++)
        TEST_ASSERT_GREATER_OR_EQUAL(duty_from_level(l - 1), duty_from_level(l));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_breathing_is_smooth);
    RUN_TEST(test_hue_cycle_is_smooth);
    RUN_TEST(test_candle_is_smooth);
    RUN_TEST(test_rgb_chase_is_smooth);
    RUN_TEST(test_candle_keeps_flickering);
    RUN_TEST(test_rotation_visits_every_pattern_in_order);
    RUN_TEST(test_duty_respects_brightness_cap);
    return UNITY_END();
}
