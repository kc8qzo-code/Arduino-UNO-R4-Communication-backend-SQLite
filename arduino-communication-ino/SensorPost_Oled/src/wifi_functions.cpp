#include "wifi_functions.h"

void serviceHttpRequest() {
  if (httpRequestState == HTTP_IDLE) return;

  if (httpRequestState == HTTP_READY) {
    if (!wifiClient.connect(SERVER_HOST, SERVER_PORT)) {
      Serial.println(F("[HTTP] Connection failed"));
      wifiClient.stop();
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

  int availableBytes = wifiClient.available();

  if (availableBytes == 0 && !wifiClient.connected()) {
    Serial.println(F("[HTTP] Client is no longer active; request cancelled"));
    wifiClient.stop();
    pendingHttpBody = "";
    httpResponse = "";
    httpRequestState = HTTP_IDLE;
    errorCount++;
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

  Serial.print(F("[HTTP] Status : "));
  Serial.println(statusCode);

  if (!timedOut && statusCode >= 200 && statusCode < 300) {
    Serial.println(F("[HTTP] Saved to database"));
    successPostCount++;
  } else {
    Serial.println(F("[HTTP] Server error - check backend logs"));
    errorCount++;
  }

  pendingHttpBody = "";
  httpResponse = "";
  httpRequestState = HTTP_IDLE;
}

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
    Serial.print(F("[WiFi] Connected  IP: "));
    Serial.print(WiFi.localIP());
    Serial.print(F("  RSSI: "));
    Serial.print(WiFi.RSSI());
    Serial.println(F(" dBm"));
  } else {
    Serial.println(F("[WiFi] Failed to connect - will retry next cycle"));
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
