#include <stdio.h>
#include <stdlib.h>
#include <unity.h>

#include "config.h"
#include "lamp.h"

#define FRAME_MS 4
#define MAX_STEP 3

static Lamp lamp;
static Rgb prev;
static uint32_t now_ms;

void setUp(void)
{
    lamp_init(&lamp);
    prev = (Rgb){0, 0, 0};
    now_ms = 0;
}

void tearDown(void) {}

static int largest_step(Rgb a, Rgb b)
{
    int d = abs(a.r - b.r);
    if (abs(a.g - b.g) > d) d = abs(a.g - b.g);
    if (abs(a.b - b.b) > d) d = abs(a.b - b.b);
    return d;
}

// Runs for ms, failing on any visible jump between frames.
static void run_smooth(uint32_t ms)
{
    for (uint32_t end = now_ms + ms; now_ms < end; now_ms += FRAME_MS) {
        Rgb c;
        lamp_render(&lamp, FRAME_MS, &c);
        int d = largest_step(c, prev);
        if (d > MAX_STEP) {
            char msg[64];
            snprintf(msg, sizeof msg, "jump of %d at t=%lu ms", d, (unsigned long)now_ms);
            TEST_FAIL_MESSAGE(msg);
        }
        prev = c;
    }
}

static void assert_dark(void)
{
    TEST_ASSERT_EQUAL_UINT8(0, prev.r);
    TEST_ASSERT_EQUAL_UINT8(0, prev.g);
    TEST_ASSERT_EQUAL_UINT8(0, prev.b);
}

static void test_stays_dark_until_switched_on(void)
{
    run_smooth(5000);
    assert_dark();
}

static void test_switch_off_fades_to_dark(void)
{
    lamp_switch_on(&lamp, PATTERN_HUE_CYCLE);
    run_smooth(5000);
    TEST_ASSERT_GREATER_THAN(200, prev.r + prev.g + prev.b);
    lamp_switch_off(&lamp);
    run_smooth(FADE_OUT_MS + FRAME_MS);
    assert_dark();
}

static void test_rapid_toggling_is_smooth(void)
{
    // Every Pattern, toggled at awkward moments: mid fade-in, mid fade-out.
    static const uint32_t holds[] = {300, 40, 1500, 200, 30, 900, 3000, 60};
    for (uint8_t i = 0; i < 16; i++) {
        lamp_switch_on(&lamp, i % PATTERN_COUNT);
        run_smooth(holds[i % 8]);
        lamp_switch_off(&lamp);
        run_smooth(holds[(i + 3) % 8]);
    }
    run_smooth(FADE_OUT_MS);
    assert_dark();
}

static void test_switch_off_while_off_is_ignored(void)
{
    lamp_switch_off(&lamp);
    run_smooth(1000);
    assert_dark();
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_stays_dark_until_switched_on);
    RUN_TEST(test_switch_off_fades_to_dark);
    RUN_TEST(test_rapid_toggling_is_smooth);
    RUN_TEST(test_switch_off_while_off_is_ignored);
    return UNITY_END();
}
