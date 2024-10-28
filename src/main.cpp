#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <LittleFS.h>
#include <RTClib.h>
#include <SPI.h>
#include <WiFi.h>

#include "functions.h"
#include "variables.h"

void setup() {
  Serial.begin(115200);
  JsonDocument config = loadConfiguration();
  char ssid[32];
  char password[64];

  if (!handleWiFiStation(ssid, sizeof(ssid), password, sizeof(password),
                         config)) {
    handleAP(ssid, sizeof(ssid), password, sizeof(password), config);
  }

  rtcFailed = !handleRTC();
  handleWebServer();
  handleMDNS();
}

void loop() {
}
