#include "date_functions.h"

String buildReadableDate(const DateTime &dateTime) {
  auto twoDigits = [](uint8_t value) {
    return (value < 10) ? String("0") + String(value) : String(value);
  };

  String readableDate = "";
  readableDate += twoDigits(dateTime.month()) + "/" + twoDigits(dateTime.day()) + "/" + String(dateTime.year()) + " ";
  readableDate += twoDigits(dateTime.hour()) + ":" + twoDigits(dateTime.minute()) + ":" + twoDigits(dateTime.second());
  return readableDate;
}

String buildUTCDate(const DateTime &dateTime) {
  // RTClib's DateTime has one-second resolution. Use the uptime clock for
  // the fractional part; the RTC is expected to contain UTC when adding Z.
  const unsigned long milliseconds = millis() % 1000UL;
  char utcDate[25];

  snprintf(utcDate, sizeof(utcDate),
           "%04u-%02u-%02uT%02u:%02u:%02u.%03luZ",
           static_cast<unsigned int>(dateTime.year()),
           static_cast<unsigned int>(dateTime.month()),
           static_cast<unsigned int>(dateTime.day()),
           static_cast<unsigned int>(dateTime.hour()),
           static_cast<unsigned int>(dateTime.minute()),
           static_cast<unsigned int>(dateTime.second()),
           milliseconds);

  return String(utcDate);
}
