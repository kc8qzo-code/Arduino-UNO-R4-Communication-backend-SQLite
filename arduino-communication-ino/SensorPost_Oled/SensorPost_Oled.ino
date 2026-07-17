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
#include "oled_functions.h"
#include "rgb_led_functions.h"
#include "arduino_uno_matrix.h"

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

// Timing configuration
const unsigned long STEP_TIME = 3;  // Time per color step 19 ms (approx 785 steps total)

// ── DHT22 ─────────────────────────────────────────────────────────────────────
#define DHT_PIN 4
#define DHT_TYPE DHT11
DHT dht(DHT_PIN, DHT_TYPE);

// ── Globals ───────────────────────────────────────────────────────────────────
WiFiClient wifiClient;

enum HttpRequestState { HTTP_IDLE,
                        HTTP_READY,
                        HTTP_READING };
HttpRequestState httpRequestState = HTTP_IDLE;
String pendingHttpBody;
String httpResponse;
unsigned long httpRequestStarted = 0;
const unsigned long HTTP_TIMEOUT_MS = 5000UL;

unsigned long lastPostTime = 0;
unsigned long lastVersionPostTime = 0;
unsigned long lastStepTime = 0;
unsigned long successPostCount = 0;
unsigned long errorCount = 0;
unsigned long postCount = 0;

int colorState = 0;  // Current and target RGB values
int currentR = 255, currentG = 0, currentB = 0;
int targetR = 255, targetG = 0, targetB = 0;

RTC_DS3231 rtc;

// ═══════════════════════════════════════════════════════════════════════════════
void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000)
    ;

  printBanner();
  dht.begin();
  delay(2000);  // DHT11 needs ~2 s after power-on before first reliable read

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

  updateRgbLed(currentR, currentG, currentB);
  delay(200);
}

// ═══════════════════════════════════════════════════════════════════════════════
void loop() {
  // Reconnect Wi-Fi if dropped
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println(F("\n[WiFi] Connection lost – reconnecting…"));
    connectWiFi();
    return;
  }

  unsigned long currentMillis = millis();

  if (currentMillis - lastStepTime >= STEP_TIME) {
    lastStepTime = currentMillis;

    // Smoothly transition current color toward target
    if (currentR < targetR) currentR++;
    else if (currentR > targetR) currentR--;

    if (currentG < targetG) currentG++;
    else if (currentG > targetG) currentG--;

    if (currentB < targetB) currentB++;
    else if (currentB > targetB) currentB--;

    // Apply color to the LED pins
    updateRgbLed(currentR, currentG, currentB);

    // If target is reached, transition to the next state
    if (currentR == targetR && currentG == targetG && currentB == targetB) {
      colorState = (colorState + 1) % 6;  // Cycle through 6 color transitions
      setNextTargetColor();
    }
  }

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

  String dateTime = buildDateTime(now);
  Serial.println(dateTime);

  JsonDocument doc;
  doc["temperature"] = round2(temperature);
  doc["humidity"] = round2(humidity);
  doc["light"] = lightOhms;
  doc["passValue"] = postCount;
  doc["dateValue"] = dateTime;

  serializeJson(doc, pendingHttpBody);
  httpRequestState = HTTP_READY;

  // ── 3. HTTP POST ───────────────────────────────────────────────────────────
  Serial.println(F("\n────────────────────────────"));
  Serial.print(F("[POST] → "));
  Serial.println(pendingHttpBody);

  updateOled(temperature, humidity, lightOhms, postCount, dateTime);
}

void serviceHttpRequest() {
  if (httpRequestState == HTTP_IDLE) return;

  if (httpRequestState == HTTP_READY) {
    if (!wifiClient.connect(SERVER_HOST, SERVER_PORT)) {
      Serial.println(F("[HTTP] Connection failed"));
      errorCount++;
      pendingHttpBody = "";
      httpRequestState = HTTP_IDLE;
      return;
    }

    wifiClient.print(F("POST "));
    wifiClient.print(API_PATH);
    wifiClient.println(F(" HTTP/1.1"));
    wifiClient.print(F("Host: "));
    wifiClient.println(SERVER_HOST);
    wifiClient.println(F("Content-Type: application/json"));
    wifiClient.print(F("Content-Length: "));
    wifiClient.println(pendingHttpBody.length());
    wifiClient.print(F("X-Device-Id: "));
    wifiClient.println(DEVICE_ID);
    wifiClient.println(F("Connection: close"));
    wifiClient.println();
    wifiClient.print(pendingHttpBody);

    httpResponse = "";
    httpRequestStarted = millis();
    httpRequestState = HTTP_READING;
    return;
  }

  byte bytesRead = 0;
  while (wifiClient.available() && bytesRead < 64) {
    char c = wifiClient.read();
    if (httpResponse.length() < 1024) httpResponse += c;
    bytesRead++;
  }

  bool timedOut = millis() - httpRequestStarted >= HTTP_TIMEOUT_MS;
  if (wifiClient.connected() && !timedOut) return;

  wifiClient.stop();
  int firstSpace = httpResponse.indexOf(' ');
  int statusCode = firstSpace >= 0
                     ? httpResponse.substring(firstSpace + 1, firstSpace + 4).toInt()
                     : -1;

  // ── 4. Response ────────────────────────────────────────────────────────────
  Serial.print(F("[HTTP] Status : "));
  Serial.println(statusCode);

  if (!timedOut && statusCode >= 200 && statusCode < 300) {
    Serial.println(F("[HTTP] ✔ Saved to database"));
    successPostCount++;
  } else {
    Serial.println(F("[HTTP] ✘ Server error – check backend logs"));
    errorCount++;
  }

  pendingHttpBody = "";
  httpResponse = "";
  httpRequestState = HTTP_IDLE;
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Wi-Fi connection (blocking with timeout)
// ═══════════════════════════════════════════════════════════════════════════════
void connectWiFi() {
  Serial.print(F("[WiFi] Connecting to "));
  Serial.print(WIFI_SSID);

  WiFi.disconnect();
  delay(200);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 40) {
    delay(500);
    Serial.print('.');
    attempts++;
  }

  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print(F("[WiFi] ✔ Connected  IP: "));
    Serial.print(WiFi.localIP());
    Serial.print(F("  RSSI: "));
    Serial.print(WiFi.RSSI());
    Serial.println(F(" dBm"));
  } else {
    Serial.println(F("[WiFi] ✘ Failed to connect – will retry next cycle"));
  }
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Helpers
// ═══════════════════════════════════════════════════════════════════════════════
float round2(float val) {
  return roundf(val * 100.0f) / 100.0f;
}

String buildDateTime(const DateTime &dt) {
  auto two = [](uint8_t v) {
    return (v < 10) ? String("0") + String(v) : String(v);
  };
  String s = "";
  s += two(dt.month()) + "/" + two(dt.day()) + "/" + String(dt.year()) + " ";
  s += two(dt.hour()) + ":" + two(dt.minute()) + ":" + two(dt.second());
  return s;
}

// State machine to define the next color to fade into
void setNextTargetColor() {
  switch (colorState) {
    case 0:  // Red -> Yellow
      targetR = 255;
      targetG = 255;
      targetB = 0;
      break;
    case 1:  // Yellow -> Green
      targetR = 0;
      targetG = 255;
      targetB = 0;
      break;
    case 2:  // Green -> Cyan
      targetR = 0;
      targetG = 255;
      targetB = 255;
      break;
    case 3:  // Cyan -> Blue
      targetR = 0;
      targetG = 0;
      targetB = 255;
      break;
    case 4:  // Blue -> Magenta
      targetR = 255;
      targetG = 0;
      targetB = 255;
      break;
    case 5:  // Magenta -> Red
      targetR = 255;
      targetG = 0;
      targetB = 0;
      break;
  }
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
