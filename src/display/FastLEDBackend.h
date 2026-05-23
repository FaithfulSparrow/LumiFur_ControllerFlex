#pragma once

#include "LumiDisplay.h"

#if LF_DISPLAY_BACKEND == 2

// ── FastLEDBackend ────────────────────────────────────────────────
// Uses the inherited GFXcanvas16 framebuffer.  show() iterates the
// buffer, converts RGB565 → CRGB, runs XY(x,y) mapping, writes into
// leds[], then calls FastLED.show().  Only begin(), show(), and
// setBrightness8() need overriding.

#define FASTLED_INTERNAL
#define FASTLED_ALLOW_INTERRUPTS 1

#include <FastLED.h>

// Flex-panel compile-time defines (overridable via PIO build_flags).
#ifndef LF_FLEX_WIDTH
#define LF_FLEX_WIDTH  48
#endif
#ifndef LF_FLEX_HEIGHT
#define LF_FLEX_HEIGHT 12
#endif

#ifndef LF_LED_DATA_PIN
#define LF_LED_DATA_PIN  14  // OE_PIN; confirm at bring-up
#endif
#ifndef LF_MAX_POWER_MA
#define LF_MAX_POWER_MA   2000
#endif

// Serpentine / flip / row-major flags are consumed by XYMap.h.
#include "XYMap.h"

class FastLEDBackend : public LumiDisplay
{
public:
    FastLEDBackend(int w, int h);

    bool begin() override;
    void setBrightness8(uint8_t b) override;
    void show() override;

private:
    CRGB *leds_;
};

#endif // LF_DISPLAY_BACKEND == 2
