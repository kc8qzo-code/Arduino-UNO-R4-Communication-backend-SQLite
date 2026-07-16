#include "arduino_uno_matrix.h"

#include <ArduinoGraphics.h>
#include <Arduino_LED_Matrix.h>

namespace {
ArduinoLEDMatrix matrix;
}

void initializeMatrix() {
  matrix.begin();
}

void updateMatrix(const String& text) {
  matrix.beginDraw();
  matrix.stroke(0x0000FF);
  matrix.textScrollSpeed(75);
  matrix.textFont(Font_5x7);
  matrix.beginText(0, 1, 0xFFFFFF);
  matrix.println(text);
  matrix.endText(SCROLL_LEFT);
  matrix.endDraw();
}
