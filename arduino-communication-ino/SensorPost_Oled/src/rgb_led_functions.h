#ifndef RGB_LED_FUNCTIONS_H
#define RGB_LED_FUNCTIONS_H

#include <Arduino.h>

bool initializeRgbLed();
void updateRgbLed(int r, int g, int b);
void slowFadeRgbColors(unsigned long currentMillis);

#endif
