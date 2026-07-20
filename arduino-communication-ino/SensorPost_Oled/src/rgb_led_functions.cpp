#include "rgb_led_functions.h"

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

namespace {
constexpr int RED_PIN = 11;
constexpr int GREEN_PIN = 10;
constexpr int BLUE_PIN = 9;
constexpr unsigned long STEP_TIME_MS = 3UL;

unsigned long lastStepTime = 0;
int colorState = 0;
int currentR = 255;
int currentG = 0;
int currentB = 0;
int targetR = 255;
int targetG = 0;
int targetB = 0;

void setNextTargetColor() {
  switch (colorState) {
    case 0:  // Red -> Yellow
      targetR = 255;
      targetG = 255;
      targetB = 0;
      break;
    case 1:  // Yellow -> Green
      targetR = 0;
      targetG = 255;
      targetB = 0;
      break;
    case 2:  // Green -> Cyan
      targetR = 0;
      targetG = 255;
      targetB = 255;
      break;
    case 3:  // Cyan -> Blue
      targetR = 0;
      targetG = 0;
      targetB = 255;
      break;
    case 4:  // Blue -> Magenta
      targetR = 255;
      targetG = 0;
      targetB = 255;
      break;
    case 5:  // Magenta -> Red
      targetR = 255;
      targetG = 0;
      targetB = 0;
      break;
  }
}
}

bool initializeRgbLed() {
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  updateRgbLed(currentR, currentG, currentB);
  return true;
}

void updateRgbLed(int r, int g, int b) {
  analogWrite(RED_PIN, r);
  analogWrite(GREEN_PIN, g);
  analogWrite(BLUE_PIN, b);
}

void slowFadeRgbColors(unsigned long currentMillis) {
  if (currentMillis - lastStepTime < STEP_TIME_MS) return;

  lastStepTime = currentMillis;

  if (currentR < targetR) currentR++;
  else if (currentR > targetR) currentR--;

  if (currentG < targetG) currentG++;
  else if (currentG > targetG) currentG--;

  if (currentB < targetB) currentB++;
  else if (currentB > targetB) currentB--;

  updateRgbLed(currentR, currentG, currentB);

  if (currentR == targetR && currentG == targetG && currentB == targetB) {
    colorState = (colorState + 1) % 6;
    setNextTargetColor();
  }
}
