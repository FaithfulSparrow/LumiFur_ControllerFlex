#pragma once

#include <Adafruit_GFX.h>
#include <gfxfont.h>

// ── LumiDisplay ─────────────────────────────────────────────────
// Abstract base class for all display backends.
// Inherits GFXcanvas16 so that backends that need a framebuffer
// (FastLED) get a free RGB565 canvas + every GFX method.  Backends
// that don't need the buffer (HUB75) override the GFX methods to
// forward directly to the native driver and ignore the canvas.
//
// External code accesses the display exclusively through this type,
// so nothing outside src/display/ knows about HUB75 or FastLED.

class LumiDisplay : public GFXcanvas16
{
public:
    LumiDisplay(int w, int h) : GFXcanvas16(w, h) {}

    virtual ~LumiDisplay() = default;

    virtual bool begin() = 0;
    virtual void setBrightness8(uint8_t b) = 0;
    virtual void show() = 0;

    // ── Adafruit_GFX virtuals that GFXcanvas16 may shadow ───
    // Re-declaring them as virtual here ensures HUB75Backend can
    // override them even when GFXcanvas16 provides non-virtual
    // implementations that hide the base class virtuals.
    virtual void drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) { GFXcanvas16::drawCircle(x0, y0, r, color); }
    virtual void fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) { GFXcanvas16::fillCircle(x0, y0, r, color); }
    virtual void setCursor(int16_t x, int16_t y) { GFXcanvas16::setCursor(x, y); }
    virtual void setTextColor(uint16_t c) { GFXcanvas16::setTextColor(c); }
    virtual void setTextColor(uint16_t c, uint16_t bg) { GFXcanvas16::setTextColor(c, bg); }
    virtual void setTextSize(uint8_t s) { GFXcanvas16::setTextSize(s); }
    virtual void setTextWrap(boolean w) { GFXcanvas16::setTextWrap(w); }
    virtual void setFont(const GFXfont *f) { GFXcanvas16::setFont(f); }
    virtual int16_t width() const { return GFXcanvas16::width(); }
    virtual int16_t height() const { return GFXcanvas16::height(); }

    // RGB565 conversion — equivalent to HUB75 library's color565(),
    // implemented here so effects that call dma_display->color565()
    // compile regardless of the display backend.
    static uint16_t color565(uint8_t r, uint8_t g, uint8_t b)
    {
        return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
    }

    // Name-compatibility shims so that 246+ call sites that use
    // HUB75-specific method names compile unchanged.
    virtual void clearScreen() { fillScreen(0); }
    virtual void flipDMABuffer() { show(); }
    virtual void drawPixelRGB888(int16_t x, int16_t y,
                                  uint8_t r, uint8_t g, uint8_t b)
    {
        drawPixel(x, y, LumiDisplay::color565(r, g, b));
    }
};
