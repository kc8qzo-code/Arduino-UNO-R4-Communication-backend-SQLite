#include "buzzer_functions.h"
#include "pitches.h"

const int buzzerPin=5;

int shaveAndAHaircutMelody[] = {
  NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4
};

int shaveAndHaircutNoteDurations[] = {
  4, 8, 8, 4, 4, 4, 4, 4
};

void shaveAndAHaircut() {
  // iterate over the notes of the melody:
  for (int thisNote = 0; thisNote < 8; thisNote++) {
    
    int noteDuration = 1000 / shaveAndHaircutNoteDurations[thisNote];
    tone(buzzerPin, shaveAndAHaircutMelody[thisNote], noteDuration);
    
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);

    noTone(buzzerPin);
  }
}
