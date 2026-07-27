#include "oled_functions.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

namespace
{
  constexpr int SCREEN_WIDTH = 128;
  constexpr int SCREEN_HEIGHT = 64;
  constexpr int OLED_RESET = -1;
  constexpr uint8_t SCREEN_ADDRESS = 0x3C;
  constexpr unsigned long SCROLL_DURATION_MS = 2000;
  constexpr int SCROLL_STEP_PIXELS = 2;

  Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
}

bool initializeOled()
{
  // Generate the display voltage from 3.3 V internally.
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
  {
    return false;
  }

  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setTextWrap(false);
  return true;
}

void updateOled(float temp, float humidity, int light, unsigned long passCount, String date, String time)
{
  display.clearDisplay();

  display.setCursor(0, 5);
  display.println("Temp: " + String(temp) + " F");
  display.println("Humidity: " + String(humidity) + "%");
  display.println("Light: " + String(light) + " Ohms");
  display.println("Pass: " + String(passCount));
  display.println("Date: " + date);
  display.println("Time: " + time);

  display.display();
}

void scrollLeftOneScreen()
{
  constexpr int BUFFER_SIZE = SCREEN_WIDTH * SCREEN_HEIGHT / 8;
  constexpr int PAGE_COUNT = SCREEN_HEIGHT / 8;
  constexpr int FRAME_COUNT = SCREEN_WIDTH / SCROLL_STEP_PIXELS;

  uint8_t originalBuffer[BUFFER_SIZE];
  uint8_t *displayBuffer = display.getBuffer();
  memcpy(originalBuffer, displayBuffer, BUFFER_SIZE);

  const unsigned long startTime = millis();

  for (int frame = 1; frame <= FRAME_COUNT; ++frame)
  {
    const int offset = frame * SCROLL_STEP_PIXELS;

    for (int page = 0; page < PAGE_COUNT; ++page)
    {
      const int pageStart = page * SCREEN_WIDTH;

      for (int x = 0; x < SCREEN_WIDTH; ++x)
      {
        displayBuffer[pageStart + x] =
            originalBuffer[pageStart + ((x + offset) % SCREEN_WIDTH)];
      }
    }

    display.display();

    const unsigned long frameDeadline =
        startTime + (SCROLL_DURATION_MS * frame) / FRAME_COUNT;
    const long remainingTime = static_cast<long>(frameDeadline - millis());
    if (remainingTime > 0)
    {
      delay(static_cast<unsigned long>(remainingTime));
    }
  }
}
