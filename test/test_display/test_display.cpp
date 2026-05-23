// ── test_display.cpp ─────────────────────────────────────────────
// Unit tests for src/display/XYMap.h — serpentine coordinate mapping.
//
// Tests the default production configuration:
//   48×12, serpentine, row-major, no flips.
//
// Alternate configurations (non-serpentine, flipped, column-major)
// should be tested via separate PlatformIO envs with different
// -DLF_FLEX_* build flags since XY() uses compile-time constants.

#include <unity.h>

// Use default 48×12 serpentine row-major config (no overrides).
#include "display/XYMap.h"

void setUp(void) {}
void tearDown(void) {}

// ── Basic identity: top-left corner ──────────────────────────────
void test_xy_origin(void)
{
    // Row 0, column 0 → index 0
    TEST_ASSERT_EQUAL_UINT16(0, XY(0, 0));
}

// ── Row 0 (even row, non-serpentine direction) ──────────────────
void test_xy_first_row_left_to_right(void)
{
    // Row 0: indices 0..47 left-to-right
    TEST_ASSERT_EQUAL_UINT16(0,  XY(0, 0));
    TEST_ASSERT_EQUAL_UINT16(1,  XY(1, 0));
    TEST_ASSERT_EQUAL_UINT16(47, XY(47, 0));
}

// ── Row 1 (odd row, serpentine — right-to-left) ─────────────────
void test_xy_second_row_serpentine(void)
{
    // Row 1: indices 48..95, mapped right-to-left
    TEST_ASSERT_EQUAL_UINT16(95, XY(0, 1));
    TEST_ASSERT_EQUAL_UINT16(48, XY(47, 1));
}

// ── Vertical adjacency ──────────────────────────────────────────
void test_xy_vertical_adjacency(void)
{
    // Row 0 col 47 → LED 47,  Row 1 col 47 → LED 48 (adjacent!)
    TEST_ASSERT_EQUAL_UINT16(47, XY(47, 0));
    TEST_ASSERT_EQUAL_UINT16(48, XY(47, 1));

    // Row 0 col 0 → LED 0,  Row 1 col 0 → LED 95
    TEST_ASSERT_EQUAL_UINT16(0,  XY(0, 0));
    TEST_ASSERT_EQUAL_UINT16(95, XY(0, 1));
}

// ── Bottom row ───────────────────────────────────────────────────
void test_xy_last_row(void)
{
    // Row 11 is odd → serpentine (right-to-left).
    // XY(0,11) = 11*48+47 = 575, XY(47,11) = 11*48+0 = 528
    TEST_ASSERT_EQUAL_UINT16(575, XY(0,  11));
    TEST_ASSERT_EQUAL_UINT16(528, XY(47, 11));
}

// ── Corner cases ─────────────────────────────────────────────────
void test_xy_corners(void)
{
    TEST_ASSERT_EQUAL_UINT16(0,   XY(0,  0));   // top-left
    TEST_ASSERT_EQUAL_UINT16(47,  XY(47, 0));   // top-right
    TEST_ASSERT_EQUAL_UINT16(575, XY(0,  11));  // bottom-left  (odd row, serpentine)
    TEST_ASSERT_EQUAL_UINT16(528, XY(47, 11));  // bottom-right (odd row, serpentine)
}

// ── Bijection: every index mapped exactly once ───────────────────
void test_xy_bijection(void)
{
    bool seen[LF_FLEX_NUM_LEDS];
    for (uint16_t i = 0; i < LF_FLEX_NUM_LEDS; ++i) seen[i] = false;

    for (int16_t y = 0; y < LF_FLEX_HEIGHT; ++y)
    {
        for (int16_t x = 0; x < LF_FLEX_WIDTH; ++x)
        {
            uint16_t idx = XY(x, y);
            TEST_ASSERT_LESS_THAN_UINT16(LF_FLEX_NUM_LEDS, idx);
            TEST_ASSERT_FALSE_MESSAGE(seen[idx], "Duplicate index in XY()");
            seen[idx] = true;
        }
    }

    // Verify all slots are filled.
    for (uint16_t i = 0; i < LF_FLEX_NUM_LEDS; ++i)
    {
        TEST_ASSERT_TRUE_MESSAGE(seen[i], "Missing index in XY()");
    }
}

// ── Row 0 monotonic (non-serpentine, values increase with x) ────
void test_xy_row0_monotonic(void)
{
    uint16_t prev = XY(0, 0);
    for (int16_t x = 1; x < LF_FLEX_WIDTH; ++x)
    {
        uint16_t cur = XY(x, 0);
        TEST_ASSERT_GREATER_THAN_UINT16(prev, cur);
        prev = cur;
    }
}

// ── Row 1 (serpentine) values decrease with x ────────────────────
void test_xy_row1_serpentine_order(void)
{
    uint16_t prev = XY(0, 1);
    for (int16_t x = 1; x < LF_FLEX_WIDTH; ++x)
    {
        uint16_t cur = XY(x, 1);
        TEST_ASSERT_LESS_THAN_UINT16(prev, cur);
        prev = cur;
    }
}

// ── All (x,y) in bounds return indices < NUM_LEDS ────────────────
void test_xy_all_in_bounds(void)
{
    for (int16_t y = 0; y < LF_FLEX_HEIGHT; ++y)
    {
        for (int16_t x = 0; x < LF_FLEX_WIDTH; ++x)
        {
            TEST_ASSERT_LESS_THAN_UINT16(LF_FLEX_NUM_LEDS, XY(x, y));
        }
    }
}

// ── NUM_LEDS macro equals W×H ────────────────────────────────────
void test_num_leds_matches_dimensions(void)
{
    TEST_ASSERT_EQUAL_UINT16(
        (uint16_t)(LF_FLEX_WIDTH * LF_FLEX_HEIGHT),
        LF_FLEX_NUM_LEDS);
}

// ── Row coverage: each row's indices fall in [y*W, y*W+W) ──────
void test_xy_row_coverage(void)
{
    for (int16_t y = 0; y < LF_FLEX_HEIGHT; ++y)
    {
        uint16_t base = (uint16_t)(y * LF_FLEX_WIDTH);
        for (int16_t x = 0; x < LF_FLEX_WIDTH; ++x)
        {
            uint16_t idx = XY(x, y);
            TEST_ASSERT_GREATER_OR_EQUAL_UINT16(base, idx);
            TEST_ASSERT_LESS_THAN_UINT16(base + LF_FLEX_WIDTH, idx);
        }
    }
}

// ── Generic index calculation: verify manual formula ─────────────
void test_xy_manual_formula(void)
{
    // For serpentine row-major:
    //   even row y: idx = y*W + x
    //   odd  row y: idx = y*W + (W-1-x) = (y+1)*W - 1 - x
    const int16_t W = LF_FLEX_WIDTH;

    // Even rows (0, 2, 4, ...)
    for (int16_t y = 0; y < LF_FLEX_HEIGHT; y += 2)
    {
        for (int16_t x = 0; x < W; ++x)
        {
            uint16_t expected = (uint16_t)(y * W + x);
            TEST_ASSERT_EQUAL_UINT16(expected, XY(x, y));
        }
    }

    // Odd rows (1, 3, 5, ...)
    for (int16_t y = 1; y < LF_FLEX_HEIGHT; y += 2)
    {
        for (int16_t x = 0; x < W; ++x)
        {
            uint16_t expected = (uint16_t)((y + 1) * W - 1 - x);
            TEST_ASSERT_EQUAL_UINT16(expected, XY(x, y));
        }
    }
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_xy_origin);
    RUN_TEST(test_xy_first_row_left_to_right);
    RUN_TEST(test_xy_second_row_serpentine);
    RUN_TEST(test_xy_vertical_adjacency);
    RUN_TEST(test_xy_last_row);
    RUN_TEST(test_xy_corners);
    RUN_TEST(test_xy_bijection);
    RUN_TEST(test_xy_row0_monotonic);
    RUN_TEST(test_xy_row1_serpentine_order);
    RUN_TEST(test_xy_all_in_bounds);
    RUN_TEST(test_num_leds_matches_dimensions);
    RUN_TEST(test_xy_row_coverage);
    RUN_TEST(test_xy_manual_formula);

    return UNITY_END();
}
