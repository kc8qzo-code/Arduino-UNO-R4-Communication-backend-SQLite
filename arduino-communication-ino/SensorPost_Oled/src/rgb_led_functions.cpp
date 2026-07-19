#include "rgb_led_functions.h"

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

namespace {
constexpr int RED_PIN = 11;
constexpr int GREEN_PIN = 10;
constexpr int BLUE_PIN = 9;
}

bool initializeRgbLed() {
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  return true;
}

void updateRgbLed(int r, int g, int b) {
  analogWrite(RED_PIN, r);
  analogWrite(GREEN_PIN, g);
  analogWrite(BLUE_PIN, b);
}
