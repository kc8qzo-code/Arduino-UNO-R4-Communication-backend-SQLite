#include "arduino_uno_matrix.h"

#include <ArduinoGraphics.h>
#include <Arduino_LED_Matrix.h>

namespace {
ArduinoLEDMatrix matrix;
int matrixTextX = 12;
String previousText;
}

void initializeMatrix() {
  matrix.begin();
}

// Method to update on-board Matrix
void updateMatrix(const String& text) {
  if (text != previousText) {
    previousText = text;
    matrixTextX = 12;
  }

  matrix.beginDraw();
  matrix.clear();
  matrix.stroke(0x0000FF);
  matrix.textFont(Font_5x7);
  matrix.beginText(matrixTextX, 1, 0xFFFFFF);
  matrix.print(text);
  matrix.endText();
  matrix.endDraw();

  // Move one column per scheduled call. Drawing one frame returns immediately,
  // unlike endText(SCROLL_LEFT), which blocks until the whole scroll completes.
  matrixTextX--;
  const int textWidth = text.length() * 6;
  if (matrixTextX < -textWidth) matrixTextX = 12;
}
