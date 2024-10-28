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
  if (!rtcFailed) {
    DateTime now = rtc.now(); // Get the current time

    // Print the current time in a readable format
    Serial.print("Current time: ");
    Serial.print(now.year());
    Serial.print('/');
    Serial.print(now.month());
    Serial.print('/');
    Serial.print(now.day());
    Serial.print(" ");
    Serial.print(now.hour());
    Serial.print(':');
    Serial.print(now.minute());
    Serial.print(':');
    Serial.println(now.second());
  } else {
    Serial.println("RTC is not available.");
  }

  delay(1000); // Wait for a second before printing again
}
