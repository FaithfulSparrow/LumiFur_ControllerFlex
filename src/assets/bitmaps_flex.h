// ── bitmaps_flex.h ────────────────────────────────────────────────
// Flex-native hero bitmaps for the WS2812B flexible matrix.
//
// bitmaps.h now defines all original 128×32 symbols unconditionally.
// The coordinate-remap helpers in main.cpp (remapDesignX/Y) handle
// nearest-neighbor scaling for any panel size.
//
// When proper flex-native bitmaps are authored (Phase 6 offline),
// define the same symbol names here at panel-native dimensions
// (e.g. 48×12 for a 12cm×48cm flex panel).  Include this file
// BEFORE bitmaps.h to override via first-definition-wins.
//
// Use tools/bitmap_resize.py to assist with resizing:
//   python3 tools/bitmap_resize.py --input src/assets/bitmaps.h \
//       --symbol maw --width 24 --height 6

#pragma once

#include <cstdint>

#ifndef PROGMEM
#define PROGMEM
#endif

// ── Placeholder ──────────────────────────────────────────────────
// No symbols defined yet.  All bitmap symbols currently come from
// bitmaps.h (128×32 design canvas).  Uncomment and populate below
// when flex-native hero faces are ready.

// Example:
// static uint8_t maw [] PROGMEM = { ... };   // 48×? for flex panel
// static uint8_t Eye [] PROGMEM = { ... };
// static uint8_t scleraR [] PROGMEM = { ... };
// static uint8_t blush [] PROGMEM = { ... };
// etc.