#include "oled_functions.h"

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

namespace {
constexpr int SCREEN_WIDTH = 128;
constexpr int SCREEN_HEIGHT = 64;
constexpr int OLED_RESET = -1;
constexpr uint8_t SCREEN_ADDRESS = 0x3C;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
}

bool initializeOled() {
  // Generate the display voltage from 3.3 V internally.
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    return false;
  }

  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setTextWrap(false);
  return true;
}

void updateOled(float temp, float humidity, int light, unsigned long passCount) {
  display.clearDisplay();

  display.setCursor(0, 28);
  display.println("Temp: " + String(temp) + " F");
  display.println("Humidity: " + String(humidity) + "%");
  display.println("Light: " + String(light) + " Ohms");
  display.println("Pass: " + String(passCount));

  display.display();
}
