#include "HUB75Backend.h"

#if LF_DISPLAY_BACKEND == 1

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

HUB75Backend::HUB75Backend(const HUB75_I2S_CFG &cfg)
    : LumiDisplay(cfg.mx_width * cfg.chain_length, cfg.mx_height),
      panel_(nullptr),
      config_(cfg)
{
}

bool HUB75Backend::begin()
{
    panel_ = new MatrixPanel_I2S_DMA(config_);
    if (!panel_) return false;
    return panel_->begin();
}

void HUB75Backend::setBrightness8(uint8_t b)
{
    if (panel_) panel_->setBrightness8(b);
}

void HUB75Backend::show()
{
    if (panel_) panel_->flipDMABuffer();
}

void HUB75Backend::clearScreen()
{
    if (panel_) panel_->fillScreen(0);
}

void HUB75Backend::flipDMABuffer()
{
    if (panel_) panel_->flipDMABuffer();
}

void HUB75Backend::drawPixelRGB888(int16_t x, int16_t y, uint8_t r, uint8_t g, uint8_t b)
{
    if (panel_) panel_->drawPixelRGB888(x, y, r, g, b);
}

// ── GFX overrides ────────────────────────────────────────────────

void HUB75Backend::drawPixel(int16_t x, int16_t y, uint16_t color)
{
    if (panel_) panel_->drawPixel(x, y, color);
}

void HUB75Backend::drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color)
{
    if (panel_) panel_->drawFastVLine(x, y, h, color);
}

void HUB75Backend::drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color)
{
    if (panel_) panel_->drawFastHLine(x, y, w, color);
}

void HUB75Backend::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
    if (panel_) panel_->fillRect(x, y, w, h, color);
}

void HUB75Backend::fillScreen(uint16_t color)
{
    if (panel_) panel_->fillScreen(color);
}

void HUB75Backend::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
{
    if (panel_) panel_->drawLine(x0, y0, x1, y1, color);
}

void HUB75Backend::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
    if (panel_) panel_->drawRect(x, y, w, h, color);
}

void HUB75Backend::drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color)
{
    if (panel_) panel_->drawCircle(x0, y0, r, color);
}

void HUB75Backend::fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color)
{
    if (panel_) panel_->fillCircle(x0, y0, r, color);
}

void HUB75Backend::setCursor(int16_t x, int16_t y)
{
    if (panel_) panel_->setCursor(x, y);
}

void HUB75Backend::setTextColor(uint16_t c)
{
    if (panel_) panel_->setTextColor(c);
}

void HUB75Backend::setTextColor(uint16_t c, uint16_t bg)
{
    if (panel_) panel_->setTextColor(c, bg);
}

void HUB75Backend::setTextSize(uint8_t s)
{
    if (panel_) panel_->setTextSize(s);
}

void HUB75Backend::setTextWrap(boolean w)
{
    if (panel_) panel_->setTextWrap(w);
}

void HUB75Backend::setFont(const GFXfont *f)
{
    if (panel_) panel_->setFont(f);
}

size_t HUB75Backend::write(uint8_t c)
{
    if (panel_) return panel_->write(c);
    return 0;
}

int16_t HUB75Backend::width() const
{
    if (panel_) return panel_->width();
    return GFXcanvas16::width();
}

int16_t HUB75Backend::height() const
{
    if (panel_) return panel_->height();
    return GFXcanvas16::height();
}

#endif // LF_DISPLAY_BACKEND == 1
