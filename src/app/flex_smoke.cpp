// ── flex_smoke.cpp ───────────────────────────────────────────────
// Standalone bring-up sketch for a WS2812B flexible LED matrix.
// Build with: pio run -e flex_smoke -t upload
//
// Phase 1 — bare-metal RGB cycle.
// Phase 2 — walking-dot coordinate diagnostic (extends below).

#if defined(LF_FLEX_SMOKE_TEST)

#include <Arduino.h>

#define FASTLED_INTERNAL
#define FASTLED_ALLOW_INTERRUPTS 1
#include <FastLED.h>

// Panel geometry — consume XY mapping from the shared header.
#ifndef LF_FLEX_WIDTH
#define LF_FLEX_WIDTH  48
#endif
#ifndef LF_FLEX_HEIGHT
#define LF_FLEX_HEIGHT 12
#endif
#ifndef LF_LED_DATA_PIN
#define LF_LED_DATA_PIN  14
#endif
#ifndef LF_MAX_POWER_MA
#define LF_MAX_POWER_MA   2000
#endif

#include "display/XYMap.h"

static CRGB leds[LF_FLEX_NUM_LEDS];

// ── Phase selection ──────────────────────────────────────────
enum SmokePhase
{
    PHASE_RGB_CYCLE = 0,
    PHASE_RASTER_WALK,
    PHASE_CORNER_DOTS,
    PHASE_DIAGONAL,
    PHASE_COUNT
};

static SmokePhase currentPhase = PHASE_RGB_CYCLE;

// ── Phase 1: all-LED RGB cycle ───────────────────────────────
static void phaseRgbCycle()
{
    static uint8_t step = 0;
    static unsigned long lastSwitch = 0;

    const unsigned long now = millis();
    if (now - lastSwitch >= 1000)
    {
        lastSwitch = now;
        step = (step + 1) % 4;
    }

    CRGB color;
    switch (step)
    {
        case 0: color = CRGB::Red;    break;
        case 1: color = CRGB::Green;  break;
        case 2: color = CRGB::Blue;   break;
        default: color = CRGB::Black; break;
    }

    fill_solid(leds, LF_FLEX_NUM_LEDS, color);
    FastLED.show();
}

// ── Phase 2a: walking-dot raster scan ────────────────────────
static void phaseRasterWalk()
{
    static int16_t wx = 0, wy = 0;
    static unsigned long lastStep = 0;

    const unsigned long now = millis();
    if (now - lastStep >= 25)
    {
        lastStep = now;
        ++wx;
        if (wx >= LF_FLEX_WIDTH)
        {
            wx = 0;
            ++wy;
            if (wy >= LF_FLEX_HEIGHT)
                wy = 0;
        }
    }

    fill_solid(leds, LF_FLEX_NUM_LEDS, CRGB::Black);
    const uint16_t idx = XY(wx, wy);
    if (idx < LF_FLEX_NUM_LEDS)
        leds[idx] = CRGB::White;

    FastLED.show();
}

// ── Phase 2b: corner dots ────────────────────────────────────
static void phaseCornerDots()
{
    fill_solid(leds, LF_FLEX_NUM_LEDS, CRGB::Black);

    const int16_t w = LF_FLEX_WIDTH  - 1;
    const int16_t h = LF_FLEX_HEIGHT - 1;

    // Four corners: TL=Red, TR=Green, BL=Blue, BR=White
    uint16_t idx;
    idx = XY(0, 0); if (idx < LF_FLEX_NUM_LEDS) leds[idx] = CRGB::Red;
    idx = XY(w, 0); if (idx < LF_FLEX_NUM_LEDS) leds[idx] = CRGB::Green;
    idx = XY(0, h); if (idx < LF_FLEX_NUM_LEDS) leds[idx] = CRGB::Blue;
    idx = XY(w, h); if (idx < LF_FLEX_NUM_LEDS) leds[idx] = CRGB::White;

    FastLED.show();
    delay(500); // stable display; no flicker
}

// ── Phase 2c: diagonal line ──────────────────────────────────
static void phaseDiagonal()
{
    fill_solid(leds, LF_FLEX_NUM_LEDS, CRGB::Black);

    const int16_t n = (LF_FLEX_WIDTH < LF_FLEX_HEIGHT)
                           ? LF_FLEX_WIDTH
                           : LF_FLEX_HEIGHT;

    for (int16_t i = 0; i < n; ++i)
    {
        const uint16_t idx = XY(i, i);
        if (idx < LF_FLEX_NUM_LEDS)
            leds[idx] = CRGB::White;
    }

    FastLED.show();
    delay(500);
}

// ── Main setup / loop ────────────────────────────────────────

void setup()
{
    Serial.begin(115200);
    delay(1000);

    Serial.printf("\n=== flex_smoke ===\n");
    Serial.printf("Panel: %d x %d, %d LEDs, data pin %d\n",
                  LF_FLEX_WIDTH, LF_FLEX_HEIGHT, LF_FLEX_NUM_LEDS, LF_LED_DATA_PIN);
    Serial.printf("Serpentine=%d  FlipX=%d  FlipY=%d  RowMajor=%d\n",
                  LF_FLEX_SERPENTINE, LF_FLEX_FLIP_X, LF_FLEX_FLIP_Y, LF_FLEX_ROW_MAJOR);

    FastLED.addLeds<WS2812B, LF_LED_DATA_PIN, GRB>(leds, LF_FLEX_NUM_LEDS)
          .setCorrection(TypicalLEDStrip);
    FastLED.setMaxPowerInVoltsAndMilliamps(5, LF_MAX_POWER_MA);
    FastLED.setDither(0);

    Serial.println("flex_smoke initialized.");
}

void loop()
{
    static unsigned long lastPhaseSwitch = 0;
    static SmokePhase     prevPhase     = PHASE_COUNT;

    const unsigned long now = millis();

    // Auto-rotate phases every 10 s; also switch on button UP press.
    if (now - lastPhaseSwitch >= 10000)
    {
        lastPhaseSwitch = now;
        currentPhase = static_cast<SmokePhase>(
            (static_cast<int>(currentPhase) + 1) % PHASE_COUNT);
    }

#if defined(BUTTON_UP)
    if (digitalRead(BUTTON_UP) == LOW)
    {
        delay(50); // debounce
        if (digitalRead(BUTTON_UP) == LOW)
        {
            lastPhaseSwitch = now;
            currentPhase = static_cast<SmokePhase>(
                (static_cast<int>(currentPhase) + 1) % PHASE_COUNT);
            while (digitalRead(BUTTON_UP) == LOW) { delay(10); }
        }
    }
#endif

    if (currentPhase != prevPhase)
    {
        prevPhase = currentPhase;
        Serial.printf("Phase: %d\n", static_cast<int>(currentPhase));
        fill_solid(leds, LF_FLEX_NUM_LEDS, CRGB::Black);
        FastLED.show();
    }

    switch (currentPhase)
    {
        case PHASE_RGB_CYCLE:    phaseRgbCycle();    break;
        case PHASE_RASTER_WALK:  phaseRasterWalk();   break;
        case PHASE_CORNER_DOTS:  phaseCornerDots();   break;
        case PHASE_DIAGONAL:     phaseDiagonal();     break;
        default: break;
    }
}

#endif // LF_FLEX_SMOKE_TEST
