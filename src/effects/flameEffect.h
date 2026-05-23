#ifndef FLAME_EFFECT_H
#define FLAME_EFFECT_H

#include <Arduino.h>
#include <FastLED.h>

#if LF_DISPLAY_BACKEND == 2 || LF_DISPLAY_BACKEND == 1
#include "display/LumiDisplay.h"
typedef LumiDisplay DisplayOutputType;
#elif defined(VIRTUAL_PANE)
#include <ESP32-VirtualMatrixPanel-I2S-DMA.h>
typedef VirtualMatrixPanel DisplayOutputType;
#else
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
typedef MatrixPanel_I2S_DMA DisplayOutputType;
#endif

void initFlameEffect(DisplayOutputType *display);

void updateAndDrawFlameEffect();

#endif // FLAME_EFFECT_H
