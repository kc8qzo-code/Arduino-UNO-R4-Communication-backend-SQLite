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

  ── Required Libraries (Arduino Library Manager) ─────────────────────────────
  • DHT sensor library
  • ArduinoHttpClient  by Arduino      (0.6.x)
  • Adafruit_SSD1306 
  • Adafruit_GFX
  • Arduino_BuiltIn 
  • ArduinoGraphics
  • Arduino_LED_Matrix
  • Wire
  • ArduinoJson        by Benoit Blanchon (7.x)
  • WiFiS3             – bundled with "Arduino UNO R4 Boards" board package

  ── Board Package (Boards Manager) ───────────────────────────────────────────
  "Arduino UNO R4 Boards" by Arduino LLC
  ═══════════════════════════════════════════════════════════════════════════════
*/
#include <Arduino_BuiltIn.h>
#include <ArduinoGraphics.h>
#include <Arduino_LED_Matrix.h>
#include <ArduinoHttpClient.h>
#include <WiFiS3.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "arduino_secrets.h"

// ── Wi-Fi credentials ─────────────────────────────────────────────────────────
char WIFI_SSID[] = SECRET_SSID;
char WIFI_PASS[] = SECRET_PASS;

// ── REST endpoint ─────────────────────────────────────────────────────────────
const char SERVER_HOST[] = SECRET_SERVER_HOST;
const int  SERVER_PORT   = SECRET_SERVER_PORT;
const char API_PATH[]    = SECRET_API_PATH;

// ── Device identity ───────────────────────────────────────────────────────────
const char DEVICE_ID[]   = "arduino-r4-01";

// ── Post interval ─────────────────────────────────────────────────────────────
const unsigned long POST_INTERVAL_MS = 2000UL;
const unsigned long MATRIX_INTERVAL = 250UL;

// ── DHT22 ─────────────────────────────────────────────────────────────────────
#define DHT_PIN  4
#define DHT_TYPE DHT11
DHT dht(DHT_PIN, DHT_TYPE);

ArduinoLEDMatrix matrix;

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C // See datasheet for Address; 0x3C for most 128x48/128x64 displays

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ── Globals ───────────────────────────────────────────────────────────────────
WiFiClient  wifiClient;
HttpClient  http(wifiClient, SERVER_HOST, SERVER_PORT);

unsigned long lastPostTime  = 0;
unsigned long successPostCount     = 0;
unsigned long errorCount    = 0;
unsigned long postCount    = 0;

int lightOhms = 0;

// ═══════════════════════════════════════════════════════════════════════════════
void setup() {
  Serial.begin(19200);
  while (!Serial && millis() < 3000);

  printBanner();
  dht.begin();
  delay(2000);   // DHT11 needs ~2 s after power-on before first reliable read

  matrix.begin();
  
  connectWiFi(); 

   // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextWrap(false);  // Allow text to trail off edge
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
 
  if (currentMillis - lastPostTime >= POST_INTERVAL_MS) {
    lastPostTime = currentMillis;
    buildSensorData();
    printStats();
  }

  if (currentMillis - lastPostTime >= MATRIX_INTERVAL) {
    lastPostTime = currentMillis;
    updateMatrix("V2.0");
  }
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Read DHT22 and POST JSON to REST endpoint
// ═══════════════════════════════════════════════════════════════════════════════
void buildSensorData() {

  postCount++;

  // ── 1. Read DHT11 ──────────────────────────────────────────────────────────
  float humidity    = dht.readHumidity();
  float temperature = dht.readTemperature(true);
  float r_fixed = 10000.0; // 10k resistor   
  int lightOhms = analogRead(A0);
  
  // Validate – DHT22 returns NaN on read failure
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println(F("[DHT] ✘ Read failed – sensor not ready or wiring issue"));
    errorCount++;
    return;
  }

  JsonDocument doc;
  doc["temperature"] = round2(temperature);
  doc["humidity"]    = round2(humidity);
  doc["light"]  = lightOhms;
  doc["passValue"] = postCount;

  String body;
  serializeJson(doc, body);

  executeHttpRequest(doc, body);

  // ── 3. HTTP POST ───────────────────────────────────────────────────────────
  Serial.println(F("\n────────────────────────────"));
  Serial.print(F("[POST] → "));
  Serial.println(body);

  updateOled(temperature, humidity, lightOhms);
}

void executeHttpRequest(ArduinoJson::JsonDocument doc, arduino::String body){
  http.beginRequest();
  http.post(API_PATH);
  http.sendHeader("Content-Type", "application/json");
  http.sendHeader("Content-Length", String(body.length()));
  http.sendHeader("X-Device-Id", DEVICE_ID);
  http.sendHeader("Connection", "close");

  http.beginBody();
  http.print(body);
  http.endRequest();

  // ── 4. Response ────────────────────────────────────────────────────────────
  int statusCode = http.responseStatusCode();
  String response   = http.responseBody();

  Serial.print(F("[HTTP] Status : "));
  Serial.println(statusCode);

  Serial.print(F("[HTTP] Body   : "));
  Serial.println(response);

  if (statusCode >= 200 && statusCode < 300) {
    Serial.println(F("[HTTP] ✔ Saved to database"));
    successPostCount++;
  } else {
    Serial.println(F("[HTTP] ✘ Server error – check backend logs"));
    errorCount++;
  }
}

// Updates OLED 128x64
void updateOled(float temp, float humidity, int light) {
  // Clear the buffer
  display.clearDisplay();

  display.setCursor(0,28);     // Start at top-left corner
  display.println("Temp: " + String(temp) + " F");
  display.println("Humidity: " + String(humidity) + "%");
  display.println("Light: " + String(light) + " Ohms");
  display.println("Pass: " + String(postCount));

  display.display(); // Actually push the text to the screen
}

// Updates onboard Matrix on UNO R4 Board
void updateMatrix(String text){
  matrix.beginDraw();
  matrix.stroke(0x0000FF);
  matrix.textScrollSpeed(75);
  matrix.textFont(Font_5x7);
  matrix.beginText(0, 1, 0xFFFFFF); // Center text
  matrix.println(text);
  matrix.endText(SCROLL_LEFT);
  matrix.endDraw();
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
  Serial.println(F("║  Sensor : DHT22  Interval : 5 s      ║"));
  Serial.println(F("╚══════════════════════════════════════╝"));
}
