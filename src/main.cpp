#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <LittleFS.h>
#include <NTPClient.h>
#include <RTClib.h>
#include <SPI.h>
#include <WiFi.h>
#include <WiFiUdp.h>

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
  ntpFailed = !updateRTCFromNTP();
  handleWebServer();
  handleMDNS();
}

void loop() {
  if (!rtcFailed) {
    DateTime now = rtc.now();
    Serial.print("Current RTC Time: ");
    Serial.print(now.hour());
    Serial.print(":");
    Serial.print(now.minute());
    Serial.print(":");
    Serial.println(now.second());
  } else if (!ntpFailed) {
    timeClient.update();
    DateTime now = DateTime(timeClient.getEpochTime());
    Serial.print("Current NTP Time: ");
    Serial.print(now.hour());
    Serial.print(":");
    Serial.print(now.minute());
    Serial.print(":");
    Serial.println(now.second());
  } else {
    // Blink an led for error here
  }
}
