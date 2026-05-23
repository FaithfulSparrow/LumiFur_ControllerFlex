#include "FastLEDBackend.h"

#if LF_DISPLAY_BACKEND == 2

FastLEDBackend::FastLEDBackend(int w, int h)
    : LumiDisplay(w, h),
      leds_(nullptr)
{
}

bool FastLEDBackend::begin()
{
    leds_ = new CRGB[LF_FLEX_NUM_LEDS];
    if (!leds_) return false;

    memset(leds_, 0, LF_FLEX_NUM_LEDS * sizeof(CRGB));

    FastLED.addLeds<WS2812B, LF_LED_DATA_PIN, GRB>(leds_, LF_FLEX_NUM_LEDS)
          .setCorrection(TypicalLEDStrip);

    FastLED.setMaxPowerInVoltsAndMilliamps(5, LF_MAX_POWER_MA);
    FastLED.setDither(0);
    FastLED.setBrightness(255); // global brightness handled by setBrightness8()

    Serial.printf("FastLED backend initialized, W=%d H=%d N=%d\n",
                  LF_FLEX_WIDTH, LF_FLEX_HEIGHT, LF_FLEX_NUM_LEDS);
    return true;
}

void FastLEDBackend::setBrightness8(uint8_t b)
{
    FastLED.setBrightness(b);
}

void FastLEDBackend::show()
{
    if (!leds_) return;

    const int16_t w = width();
    const int16_t h = height();

    for (int16_t y = 0; y < h; ++y)
    {
        for (int16_t x = 0; x < w; ++x)
        {
            const uint16_t idx = XY(x, y);
            if (idx >= LF_FLEX_NUM_LEDS) continue;

            const uint16_t c = getBuffer()[static_cast<size_t>(y) * static_cast<size_t>(width()) + static_cast<size_t>(x)];
            leds_[idx] = CRGB(
                static_cast<uint8_t>((c & 0xF800) >> 8),                       // R: top 5 bits → top 5 bits of 8
                static_cast<uint8_t>((c & 0x07E0) >> 3),                       // G: middle 6 bits → top 6 bits of 8
                static_cast<uint8_t>((c & 0x001F) << 3)                        // B: low 5 bits → top 5 bits of 8
            );
        }
    }

    FastLED.show();
}

#endif // LF_DISPLAY_BACKEND == 2
