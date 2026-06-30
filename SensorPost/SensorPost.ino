/*
  SensorPost.ino
  ═══════════════════════════════════════════════════════════════════════════════
  Arduino Uno R4 WiFi  →  HTTP POST every 5 s  →  http://192.168.1.239:8080/api/sensors
  Sensor  : DHT22 (Temperature + Humidity)
  Backend : REST API  →  SQLite database

  ── Wiring ───────────────────────────────────────────────────────────────────
  DHT22 Pin 1 (VCC)  → 3.3 V
  DHT22 Pin 2 (DATA) → D2  +  10 kΩ pull-up resistor to 3.3 V
  DHT22 Pin 4 (GND)  → GND

  ── Required Libraries (Arduino Library Manager) ─────────────────────────────
  • DHT sensor library  by Adafruit  (1.4.x)
  • Adafruit Unified Sensor            (1.1.x)  ← DHT dependency
  • ArduinoHttpClient  by Arduino      (0.6.x)
  • ArduinoJson        by Benoit Blanchon (7.x)
  • WiFiS3             – bundled with "Arduino UNO R4 Boards" board package

  ── Board Package (Boards Manager) ───────────────────────────────────────────
  "Arduino UNO R4 Boards" by Arduino LLC
  ═══════════════════════════════════════════════════════════════════════════════
*/
#include <Arduino_BuiltIn.h>
#include <ArduinoGraphics.h>
#include <Arduino_LED_Matrix.h>
#include <WiFiS3.h>
#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>
#include "arduino_secrets.h"

// ── Wi-Fi credentials ─────────────────────────────────────────────────────────
char WIFI_SSID[] = SECRET_SSID;
char WIFI_PASS[] = SECRET_PASS;

// ── REST endpoint ─────────────────────────────────────────────────────────────
const char SERVER_HOST[] = "192.168.1.239";
const int  SERVER_PORT   = 8080;
const char API_PATH[]    = "/api/sensors";

// ── Device identity ───────────────────────────────────────────────────────────
const char DEVICE_ID[]   = "arduino-r4-01";

// ── Post interval ─────────────────────────────────────────────────────────────
const unsigned long POST_INTERVAL_MS = 5000UL;

// ── DHT22 ─────────────────────────────────────────────────────────────────────
#define DHT_PIN  4
#define DHT_TYPE DHT11
DHT dht(DHT_PIN, DHT_TYPE);

LiquidCrystal_I2C lcd(0x27, 16, 2);

ArduinoLEDMatrix matrix;

// ── Globals ───────────────────────────────────────────────────────────────────
WiFiClient  wifiClient;
HttpClient  http(wifiClient, SERVER_HOST, SERVER_PORT);
unsigned long lastPostTime  = 0;
unsigned long postCount     = 0;
unsigned long errorCount    = 0;
int lightOhms = 0;

// ═══════════════════════════════════════════════════════════════════════════════
void setup() {
  Serial.begin(9600);
  while (!Serial && millis() < 3000);

  printBanner();
  dht.begin();
  delay(2000);   // DHT11 needs ~2 s after power-on before first reliable read

  lcd.init(); 
  lcd.backlight();  //open the backlight

  matrix.begin();
  
  connectWiFi();
}

// ═══════════════════════════════════════════════════════════════════════════════
void loop() {
  // Reconnect Wi-Fi if dropped
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println(F("\n[WiFi] Connection lost – reconnecting…"));
    connectWiFi();
    return;
  }

  unsigned long now = millis();
  if (now - lastPostTime >= POST_INTERVAL_MS) {
    lastPostTime = now;
    postSensorData();
    printStats();
  }
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Read DHT22 and POST JSON to REST endpoint
// ═══════════════════════════════════════════════════════════════════════════════
void postSensorData() {
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

  String body;
  serializeJson(doc, body);

  // ── 3. HTTP POST ───────────────────────────────────────────────────────────
  Serial.println(F("\n────────────────────────────"));
  Serial.print(F("[POST] → "));
  Serial.println(body);

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
    postCount++;
  } else {
    Serial.println(F("[HTTP] ✘ Server error – check backend logs"));
    errorCount++;
  }

  updateLcd(temperature, humidity);

  updateMatrix(temperature, humidity, lightOhms); 
}


// Updates LiquidCrystal_I2C Display
void updateLcd(float temp, float humidity) {
   // Send data to LCD
  lcd.clear();
  lcd.setCursor(1, 0); // set the cursor to column 3, line 0
  lcd.print("Temp: " + String(temp) + " F");  // Print a message to the LCD

  lcd.setCursor(0, 1); // set the cursor to column 2, line 1
  lcd.print("Humidity: " + String(humidity) + "%");  // Print a message to the LCD.
}

// Updates onboard Matrix on UNO R4 Board
void updateMatrix(float temperature, float humidity, int light){
  matrix.beginDraw();

  matrix.stroke(0x0000FF);
  matrix.textScrollSpeed(75);

  // add the text
  
  String text = "Temp: " + String(temperature) + " F, " + "Humidity: " + String(humidity) + "%, " + "Light: " + String(light);
  matrix.textFont(Font_5x7);
  matrix.beginText(0, 1, 0xFFFFFF);
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
  Serial.print(postCount);
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
