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
  JsonDocument timeConfig = loadConfigurationTime();
  char ssid[32];
  char password[64];

  if (!handleWiFiStation(ssid, sizeof(ssid), password, sizeof(password),
                         config)) {
    handleAP(ssid, sizeof(ssid), password, sizeof(password), config);
  }
  String onTime = timeConfig["onTime"] | "";
  String offTime = timeConfig["offTime"] | "";
  turnOff = stringToDateTime(offTime.c_str());
  // turnOff = stringToDateTime(onTime.c_str());

  rtcFailed = !handleRTC();
  ntpFailed = !updateRTCFromNTP();
  handleWebServer();
  handleMDNS();
}

void loop() {
  DateTime now;
  if (!rtcFailed) {
    now = rtc.now();
  } else if (!ntpFailed) {
    timeClient.update();
    now = DateTime(timeClient.getEpochTime());
  } else {
    // Blink an led for error here
  }

  if ((now.hour() == turnOff.hour()) && (now.minute() == turnOff.minute())) {
    Serial.println("led off");
  }

  if ((now.hour() == turnOn.hour()) && (now.minute() == turnOn.minute())) {
    Serial.println("led on");
  }
}
