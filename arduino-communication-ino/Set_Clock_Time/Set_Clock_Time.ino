// Updated example using Adafruit RTClib for reliable DS3231 time set/read
#include <Wire.h>
#include <RTClib.h>

RTC_DS3231 rtc;

void setup() {
  Serial.begin(115200);
  while (!Serial) ;
  Serial.println("RTC Set/Read example");

  Wire.begin();
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC. Check wiring.");
    while (1) delay(10);
  }

  // Set the RTC to four hours after the sketch compile time
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)) + TimeSpan(0, 4, 0, 0));

  // Read and print current RTC time
  DateTime now = rtc.now();
  Serial.println(buildDateTime(now));
  delay(200);
}

void loop() {
  DateTime now = rtc.now();
  Serial.println(buildDateTime(now));
  delay(1000);
}

String buildDateTime(const DateTime &dt){
  auto two = [](uint8_t v){ return (v < 10) ? String("0") + String(v) : String(v); };
  String s = "";
  s += two(dt.month()) + "/" + two(dt.day()) + "/" + String(dt.year()) + " ";
  s += two(dt.hour()) + ":" + two(dt.minute()) + ":" + two(dt.second());
  return s;
}
