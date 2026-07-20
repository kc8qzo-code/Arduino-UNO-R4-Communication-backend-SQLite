/*
  Expected File
  arduino_secrets.h

  Must contain #define for
  #define SECRET_SSID "SSID For WIFI Network"
  #define SECRET_PASS "Password For SSID Above"
  #define SECRET_SERVER_HOST "IP Or DNS name for backend API"
  #define SECRET_SERVER_PORT 8080 or "Port for Backend API"
  #define SECRET_API_PATH    "/api/sensors" Or "Whatever your API path is for PUT command"

  SensorPost.ino
  ═══════════════════════════════════════════════════════════════════════════════
  Arduino Uno R4 WiFi  →  HTTP POST every 2 s  →  http://192.168.0.101:8080/api/sensors
  Sensor  : DHT22 (Temperature + Humidity)
  Backend : REST API  →  SQLite database

  ── Wiring ───────────────────────────────────────────────────────────────────
  DHT22 Pin 1 (VCC)  → 3.3 V
  DHT22 Pin 2 (DATA) → D4  +  10 kΩ pull-up resistor to 3.3 V
  DHT22 Pin 4 (GND)  → GND

  PhotoCell Pin A0

  OLED 128x64
  Pin GND → Arduino GND
  Pin VCC → Arduino 5V or 3.3
  Pin VSCL → Arduino A5 (or SCL pin)
  Pin SDA → Arduino A4 (or SDA pin)

  RGB LED PINOUT On Arduino
  Pin Red 11
  Pin Green 10
  Pin Blue 9

  DS3231 RTC Clock Module
  Pin VCC 5v
  Pin GND - GND
  Pin SDA - SDA On Arduino Parrellel with OLED
  Pin SCL - SCL On Arduino Parrellel with OLED

  Passive Buzzer
  PIN GND -> GND
  PIN + -> Pin 5 (PWM)

  ── Required Libraries (Arduino Library Manager) ─────────────────────────────
  • DHT sensor library (DHT.h)
  • ArduinoHttpClient  by Arduino      (0.6.x)
  • Adafruit_SSD1306 
  • Adafruit_BusIO
  • Adafruit_RTCLib
  • Adafruit_GFX
  • Arduino_BuiltIn 
  • ArduinoGraphics
  • Arduino_LED_Matrix
  • Wire
  • DS3231 by (by NorthernWidget)
  • ArduinoJson        by Benoit Blanchon (7.x)
  • WiFiS3             – bundled with "Arduino UNO R4 Boards" board package

  Included files
  • arduino_secrets
  • oled_functions
  • rgb_led_functions
  • arduino_uno_matrix

  ── Board Package (Boards Manager) ───────────────────────────────────────────
  "Arduino UNO R4 Boards" by Arduino LLC
  ═══════════════════════════════════════════════════════════════════════════════
*/
#include <Arduino_BuiltIn.h>
#include <WiFiS3.h>
#include <RTClib.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <Wire.h>
#include "arduino_secrets.h"
#include "src/date_functions.h"
#include "src/oled_functions.h"
#include "src/rgb_led_functions.h"
#include "src/arduino_uno_matrix.h"
#include "src/buzzer_functions.h"
#include "src/wifi_functions.h"

// ── Wi-Fi credentials ─────────────────────────────────────────────────────────
char WIFI_SSID[] = SECRET_SSID;
char WIFI_PASS[] = SECRET_PASS;

// ── REST endpoint ─────────────────────────────────────────────────────────────
const char SERVER_HOST[] = SECRET_SERVER_HOST;
const int SERVER_PORT = SECRET_SERVER_PORT;
const char API_PATH[] = SECRET_API_PATH;

// ── Device identity ───────────────────────────────────────────────────────────
const char DEVICE_ID[] = "arduino-r4-01";

// ── Post interval ─────────────────────────────────────────────────────────────
const unsigned long POST_INTERVAL_MS = 2000UL;
const unsigned long MATRIX_INTERVAL = 250UL;
const unsigned long MELODY_INTERVAL = 10000UL;  // Play melody every 10 seconds

// ── DHT22 ─────────────────────────────────────────────────────────────────────
#define DHT_PIN 4
#define DHT_TYPE DHT11
DHT dht(DHT_PIN, DHT_TYPE);

// ── Globals ───────────────────────────────────────────────────────────────────
WiFiClient wifiClient;

HttpRequestState httpRequestState = HTTP_IDLE;
String pendingHttpBody;
String httpResponse;
unsigned long httpRequestStarted = 0;
const unsigned long HTTP_TIMEOUT_MS = 5000UL;
const int HTTP_CONNECT_TIMEOUT_MS = 500;

unsigned long lastPostTime = 0;
unsigned long lastVersionPostTime = 0;
unsigned long lastMelodyTime = 0;
unsigned long successPostCount = 0;
unsigned long errorCount = 0;
unsigned long postCount = 0;

bool notPlayedMelody = true;

RTC_DS3231 rtc;

// ═══════════════════════════════════════════════════════════════════════════════
void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000);

  printBanner();
  dht.begin();
  delay(2000);  // DHT11 needs ~2 s after power-on before first reliable read

  // Keep an unavailable server from blocking an entire loop() pass.
  wifiClient.setConnectionTimeout(HTTP_CONNECT_TIMEOUT_MS);

  // Start I2C communication
  Wire.begin();
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC. Check wiring.");
    while (1) delay(10);
  }

  Serial.println("Reading DS3231 clock...");

  // Set the time and date to match your computer's compile time.
  // This automatically sets the RTC to the exact moment you upload the code.
  // rtc.adjust(DateTime(__DATE__, __TIME__));

  initializeMatrix();

  connectWiFi();

  if (!initializeOled()) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;  // Don't proceed, loop forever
  }

  if (!initializeRgbLed()) {
    Serial.println(F("RGB LED Pin Allocation Failed"));
    for (;;)
      ;  // Don't proceed, loop forever
  }

  delay(200);
}

// ═══════════════════════════════════════════════════════════════════════════════
void loop() {
  // Reconnect Wi-Fi if dropped
  if (WiFi.status() != WL_CONNECTED) {
    wifiClient.stop();
    pendingHttpBody = "";
    httpResponse = "";
    httpRequestState = HTTP_IDLE;
    Serial.println(F("\n[WiFi] Connection lost – reconnecting…"));
    connectWiFi();
    return;
  }

  unsigned long currentMillis = millis();

  slowFadeRgbColors(currentMillis);

  if (currentMillis - lastPostTime >= POST_INTERVAL_MS) {
    lastPostTime = currentMillis;
    buildSensorData();
    printStats();
  }

  serviceHttpRequest();

  if (currentMillis - lastVersionPostTime >= MATRIX_INTERVAL) {
    lastVersionPostTime += MATRIX_INTERVAL;
    updateMatrix("V2.0");
  }

  if (currentMillis - lastMelodyTime >= MELODY_INTERVAL && notPlayedMelody) {
    lastMelodyTime += MELODY_INTERVAL;
    notPlayedMelody = false;
    shaveAndAHaircut();
  }
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Read DHT22 and POST JSON to REST endpoint
// ═══════════════════════════════════════════════════════════════════════════════
void buildSensorData() {

  if (httpRequestState != HTTP_IDLE) {
    Serial.println(F("[POST] Previous request still active; sample skipped"));
    return;
  }

  postCount++;

  // ── 1. Read DHT11 and light sensor ──────────────────────────────────────────────────────────
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature(true);
  int lightOhms = analogRead(A0);

  // Read and print current RTC time
  DateTime now = rtc.now();

  // Validate – DHT22 returns NaN on read failure
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println(F("[DHT] ✘ Read failed – sensor not ready or wiring issue"));
    errorCount++;
    return;
  }

  String dateTime = buildReadableDate(now);
  Serial.println(dateTime);

  String utcStyleTime = buildUTCDate(now);
  Serial.println("UTC Type");
  Serial.println(utcStyleTime);

  JsonDocument doc;
  doc["temperature"] = round2(temperature);
  doc["humidity"] = round2(humidity);
  doc["light"] = lightOhms;
  doc["passValue"] = postCount;
  doc["sentAt"] = utcStyleTime;

  serializeJson(doc, pendingHttpBody);
  httpRequestState = HTTP_READY;

  // ── 3. HTTP POST ───────────────────────────────────────────────────────────
  Serial.println(F("\n────────────────────────────"));
  Serial.print(F("[POST] → "));
  Serial.println(pendingHttpBody);

  updateOled(temperature, humidity, lightOhms, postCount, dateTime);
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Helpers
// ═══════════════════════════════════════════════════════════════════════════════
float round2(float val) {
  return roundf(val * 100.0f) / 100.0f;
}

void printStats() {
  Serial.print(F("[STATS] Posts OK: "));
  Serial.print(successPostCount);
  Serial.print(F("  Errors: "));
  Serial.println(errorCount);
}

void printBanner() {
  Serial.println(F(""));
  Serial.println(F("╔══════════════════════════════════════╗"));
  Serial.println(F("║  Arduino Uno R4 WiFi – DHT22 Logger ║"));
  Serial.println(F("║  Target : 192.168.1.239:8080         ║"));
  Serial.println(F("║  Sensor : DHT22  Interval : 2 s      ║"));
  Serial.println(F("╚══════════════════════════════════════╝"));
}
