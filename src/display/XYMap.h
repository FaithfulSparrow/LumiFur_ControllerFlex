// ── XYMap.h ───────────────────────────────────────────────────────
// Pure-header serpentine coordinate mapping for flexible LED matrices.
// No dependencies beyond <cstdint> — safe to include in native tests.
//
// Compile-time configuration (all overridable via -D build flags):
//   LF_FLEX_WIDTH        panel width  (default 48)
//   LF_FLEX_HEIGHT       panel height (default 12)
//   LF_FLEX_SERPENTINE   1 = alternate row direction     (default 1)
//   LF_FLEX_FLIP_X       1 = mirror X axis               (default 0)
//   LF_FLEX_FLIP_Y       1 = mirror Y axis               (default 0)
//   LF_FLEX_ROW_MAJOR    1 = rows first, 0 = cols first (default 1)

#pragma once

#include <cstdint>

#ifndef LF_FLEX_WIDTH
#define LF_FLEX_WIDTH  48
#endif
#ifndef LF_FLEX_HEIGHT
#define LF_FLEX_HEIGHT 12
#endif
#ifndef LF_FLEX_SERPENTINE
#define LF_FLEX_SERPENTINE 1
#endif
#ifndef LF_FLEX_FLIP_X
#define LF_FLEX_FLIP_X  0
#endif
#ifndef LF_FLEX_FLIP_Y
#define LF_FLEX_FLIP_Y  0
#endif
#ifndef LF_FLEX_ROW_MAJOR
#define LF_FLEX_ROW_MAJOR 1
#endif

#define LF_FLEX_NUM_LEDS  (LF_FLEX_WIDTH * LF_FLEX_HEIGHT)

/// Map (x, y) pixel coordinate → linear LED index.
/// Applies serpentine routing, axis flips, and row/column-major ordering
/// as configured by the LF_FLEX_* compile-time macros.
static inline uint16_t XY(int16_t x, int16_t y)
{
#if LF_FLEX_FLIP_X
    x = (LF_FLEX_WIDTH  - 1) - x;
#endif
#if LF_FLEX_FLIP_Y
    y = (LF_FLEX_HEIGHT - 1) - y;
#endif
#if LF_FLEX_ROW_MAJOR
    if (LF_FLEX_SERPENTINE && (y & 1))
        x = (LF_FLEX_WIDTH - 1) - x;
    return (uint16_t)(y * LF_FLEX_WIDTH + x);
#else
    if (LF_FLEX_SERPENTINE && (x & 1))
        y = (LF_FLEX_HEIGHT - 1) - y;
    return (uint16_t)(x * LF_FLEX_HEIGHT + y);
#endif
}
