#pragma once

#include "LumiDisplay.h"

#if LF_DISPLAY_BACKEND == 1

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

// ── HUB75Backend ────────────────────────────────────────────────
// Wraps a native MatrixPanel_I2S_DMA instance.  Every GFX virtual
// method is overridden to forward directly to the hardware pointer,
// bypassing the GFXcanvas16 framebuffer entirely — zero extra RAM
// and zero behaviour change from the pre-abstraction codebase.

class HUB75Backend : public LumiDisplay
{
public:
    explicit HUB75Backend(const HUB75_I2S_CFG &cfg);

    bool begin() override;
    void setBrightness8(uint8_t b) override;
    void show() override;
    void clearScreen() override;
    void flipDMABuffer() override;
    void drawPixelRGB888(int16_t x, int16_t y, uint8_t r, uint8_t g, uint8_t b) override;

    // ── GFX overrides ──────────────────────────────────────
    void drawPixel(int16_t x, int16_t y, uint16_t color) override;
    void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) override;
    void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) override;
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) override;
    void fillScreen(uint16_t color) override;
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) override;
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) override;
    void drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) override;
    void fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) override;

    // Adafruit_GFX virtuals that the codebase calls
    void setCursor(int16_t x, int16_t y) override;
    void setTextColor(uint16_t c) override;
    void setTextColor(uint16_t c, uint16_t bg) override;
    void setTextSize(uint8_t s) override;
    void setTextWrap(boolean w) override;
    void setFont(const GFXfont *f) override;

    size_t write(uint8_t c) override;

    // width() / height() queries — forwarded for safety
    int16_t width() const override;
    int16_t height() const override;

private:
    MatrixPanel_I2S_DMA *panel_;
    HUB75_I2S_CFG config_;
};

#endif // LF_DISPLAY_BACKEND == 1
