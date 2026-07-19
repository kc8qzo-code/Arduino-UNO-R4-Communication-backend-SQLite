#ifndef DATE_FUNCTIONS_H
#define DATE_FUNCTIONS_H

#include <Arduino.h>
#include <RTClib.h>

String buildReadableDate(const DateTime &dateTime);
String buildUTCDate(const DateTime &dateTime);

#endif
