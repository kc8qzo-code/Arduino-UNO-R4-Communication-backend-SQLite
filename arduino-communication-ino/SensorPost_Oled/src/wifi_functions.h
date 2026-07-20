#ifndef WIFI_FUNCTIONS_H
#define WIFI_FUNCTIONS_H

#include <Arduino.h>
#include <WiFiS3.h>

enum HttpRequestState {
  HTTP_IDLE,
  HTTP_READY,
  HTTP_READING
};

extern char WIFI_SSID[];
extern char WIFI_PASS[];
extern const char SERVER_HOST[];
extern const int SERVER_PORT;
extern const char API_PATH[];
extern const char DEVICE_ID[];

extern WiFiClient wifiClient;
extern HttpRequestState httpRequestState;
extern String pendingHttpBody;
extern String httpResponse;
extern unsigned long httpRequestStarted;
extern const unsigned long HTTP_TIMEOUT_MS;
extern unsigned long successPostCount;
extern unsigned long errorCount;

void serviceHttpRequest();
void connectWiFi();

#endif
