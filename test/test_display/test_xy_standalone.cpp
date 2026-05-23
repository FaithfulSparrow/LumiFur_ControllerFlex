// ── test_xy_standalone.cpp ────────────────────────────────────────
// Standalone validation of XY() coordinate mapping.
// Compile: g++ -std=c++11 -Isrc -o /tmp/test_xy test/test_display/test_xy_standalone.cpp && /tmp/test_xy
// No Unity framework dependency — pure assert().

#include <cstdio>
#include <cstdlib>
#include <stdint.h>

// Use default 48×12 serpentine row-major config.
#include "display/XYMap.h"

static int failures = 0;

#define CHECK(cond, ...)                                          \
    do {                                                          \
        if (!(cond)) {                                            \
            printf("FAIL: " __VA_ARGS__);                         \
            printf("  (%s:%d)\n", __FILE__, __LINE__);           \
            ++failures;                                           \
        }                                                         \
    } while (0)

#define CHECK_EQ(expected, actual, ...)                           \
    do {                                                          \
        uint16_t e = (expected);                                  \
        uint16_t a = (actual);                                    \
        if (e != a) {                                             \
            printf("FAIL: " __VA_ARGS__);                         \
            printf("  expected=%u actual=%u (%s:%d)\n",           \
                   e, a, __FILE__, __LINE__);                     \
            ++failures;                                           \
        }                                                         \
    } while (0)

static void test_xy_origin()
{
    CHECK_EQ(0, XY(0, 0), "XY(0,0) != 0");
}

static void test_xy_first_row()
{
    CHECK_EQ(0,  XY(0, 0),  "XY(0,0)");
    CHECK_EQ(1,  XY(1, 0),  "XY(1,0)");
    CHECK_EQ(47, XY(47, 0), "XY(47,0)");
}

static void test_xy_second_row_serpentine()
{
    CHECK_EQ(95, XY(0, 1),  "XY(0,1)");
    CHECK_EQ(48, XY(47, 1), "XY(47,1)");
}

static void test_xy_vertical_adjacency()
{
    CHECK_EQ(47, XY(47, 0), "XY(47,0)");
    CHECK_EQ(48, XY(47, 1), "XY(47,1) -> should be adjacent to 47");
    CHECK_EQ(0,  XY(0, 0),  "XY(0,0)");
    CHECK_EQ(95, XY(0, 1),  "XY(0,1)");
}

static void test_xy_last_row()
{
    // Row 11 is odd → serpentine (right-to-left).
    // XY(0,11) = 11*48+47 = 575  (bottom-left = end of strip)
    // XY(47,11)= 11*48+0  = 528  (bottom-right)
    CHECK_EQ(575, XY(0,  11), "XY(0,11) — bottom-left, serpentine");
    CHECK_EQ(528, XY(47, 11), "XY(47,11) — bottom-right, serpentine");
}

static void test_xy_corners()
{
    CHECK_EQ(0,   XY(0,  0),  "top-left");
    CHECK_EQ(47,  XY(47, 0),  "top-right");
    CHECK_EQ(575, XY(0,  11), "bottom-left  (odd row, serpentine)");
    CHECK_EQ(528, XY(47, 11), "bottom-right (odd row, serpentine)");
}

static void test_xy_bijection()
{
    bool seen[LF_FLEX_NUM_LEDS];
    for (uint16_t i = 0; i < LF_FLEX_NUM_LEDS; ++i) seen[i] = false;

    for (int16_t y = 0; y < LF_FLEX_HEIGHT; ++y)
    {
        for (int16_t x = 0; x < LF_FLEX_WIDTH; ++x)
        {
            uint16_t idx = XY(x, y);
            CHECK(idx < LF_FLEX_NUM_LEDS,
                  "XY(%d,%d)=%u >= NUM_LEDS=%u",
                  x, y, idx, LF_FLEX_NUM_LEDS);
            if (idx < LF_FLEX_NUM_LEDS)
            {
                CHECK(!seen[idx],
                      "Duplicate index %u at XY(%d,%d)", idx, x, y);
                seen[idx] = true;
            }
        }
    }

    for (uint16_t i = 0; i < LF_FLEX_NUM_LEDS; ++i)
    {
        CHECK(seen[i], "Missing index %u from XY()", i);
    }
}

static void test_xy_row0_monotonic()
{
    uint16_t prev = XY(0, 0);
    for (int16_t x = 1; x < LF_FLEX_WIDTH; ++x)
    {
        uint16_t cur = XY(x, 0);
        CHECK(cur > prev,
              "Row 0 not monotonic: XY(%d,0)=%u <= XY(%d,0)=%u",
              x, cur, x-1, prev);
        prev = cur;
    }
}

static void test_xy_row1_serpentine()
{
    uint16_t prev = XY(0, 1);
    for (int16_t x = 1; x < LF_FLEX_WIDTH; ++x)
    {
        uint16_t cur = XY(x, 1);
        CHECK(cur < prev,
              "Row 1 serpentine wrong: XY(%d,1)=%u >= XY(%d,1)=%u",
              x, cur, x-1, prev);
        prev = cur;
    }
}

static void test_xy_manual_formula()
{
    const int16_t W = LF_FLEX_WIDTH;

    // Even rows: y*W + x
    for (int16_t y = 0; y < LF_FLEX_HEIGHT; y += 2)
    {
        for (int16_t x = 0; x < W; ++x)
        {
            uint16_t expected = (uint16_t)(y * W + x);
            CHECK_EQ(expected, XY(x, y),
                     "Even row formula failed at (%d,%d)", x, y);
        }
    }

    // Odd rows: (y+1)*W - 1 - x
    for (int16_t y = 1; y < LF_FLEX_HEIGHT; y += 2)
    {
        for (int16_t x = 0; x < W; ++x)
        {
            uint16_t expected = (uint16_t)((y + 1) * W - 1 - x);
            CHECK_EQ(expected, XY(x, y),
                     "Odd row serpentine formula failed at (%d,%d)", x, y);
        }
    }
}

static void test_xy_all_in_bounds()
{
    for (int16_t y = 0; y < LF_FLEX_HEIGHT; ++y)
    {
        for (int16_t x = 0; x < LF_FLEX_WIDTH; ++x)
        {
            uint16_t idx = XY(x, y);
            CHECK(idx < LF_FLEX_NUM_LEDS,
                  "XY(%d,%d)=%u >= %u", x, y, idx, LF_FLEX_NUM_LEDS);
        }
    }
}

static void test_num_leds_macro()
{
    CHECK_EQ((uint16_t)(LF_FLEX_WIDTH * LF_FLEX_HEIGHT),
             LF_FLEX_NUM_LEDS,
             "LF_FLEX_NUM_LEDS != W×H");
}

static void test_row_coverage()
{
    const int16_t W = LF_FLEX_WIDTH;
    for (int16_t y = 0; y < LF_FLEX_HEIGHT; ++y)
    {
        uint16_t base = (uint16_t)(y * W);
        for (int16_t x = 0; x < W; ++x)
        {
            uint16_t idx = XY(x, y);
            CHECK(idx >= base && idx < base + W,
                  "Row %d: XY(%d,%d)=%u not in [%u, %u)",
                  y, x, y, idx, base, base + W);
        }
    }
}

int main()
{
    printf("=== XY() coordinate mapping tests ===\n");
    printf("Config: %dx%d, serpentine=%d, row_major=%d, flip_x=%d, flip_y=%d\n",
           LF_FLEX_WIDTH, LF_FLEX_HEIGHT,
           LF_FLEX_SERPENTINE, LF_FLEX_ROW_MAJOR,
           LF_FLEX_FLIP_X, LF_FLEX_FLIP_Y);

    test_xy_origin();
    test_xy_first_row();
    test_xy_second_row_serpentine();
    test_xy_vertical_adjacency();
    test_xy_last_row();
    test_xy_corners();
    test_xy_bijection();
    test_xy_row0_monotonic();
    test_xy_row1_serpentine();
    test_xy_manual_formula();
    test_xy_all_in_bounds();
    test_num_leds_macro();
    test_row_coverage();

    if (failures == 0)
    {
        printf("\n=== ALL TESTS PASSED ===\n");
        return 0;
    }
    else
    {
        printf("\n=== %d TEST(S) FAILED ===\n", failures);
        return 1;
    }
}