#ifndef OLED_FUNCTIONS_H
#define OLED_FUNCTIONS_H

#include <Arduino.h>

bool initializeOled();
void updateOled(float temp, float humidity, int light, unsigned long passCount);

#endif
