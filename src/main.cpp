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
  pinMode(errorLedPin, OUTPUT);
  pinMode(relayPin, OUTPUT);
  digitalWrite(errorLedPin, LOW);
  digitalWrite(relayPin, LOW);

  JsonDocument config = loadConfiguration();
  serializeJson(config, Serial);

  if (config.containsKey("onTime") && config.containsKey("offTime")) {
    validOnOffTimes = true;
    turnOn = DateTime((unsigned int)config["onTime"]);
    turnOff = DateTime((unsigned int)config["offTime"]);
  } else {
    validOnOffTimes = false;
  }

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
  if (ntpFailed && rtcFailed) {
    blinkErrorLed();
  }
}

void loop() {
  if (validOnOffTimes) {
    DateTime now;
    if (!ntpFailed) {
      now = DateTime(timeClient.getEpochTime());
    } else if (!rtcFailed) {
      now = rtc.now();
    } else {
      return;
    }
    if (now.hour() == turnOn.hour() && now.minute() == turnOn.minute()) {
      isOn = true;
    } else if (now.hour() == turnOff.hour() &&
               now.minute() == turnOff.minute()) {
      isOn = false;
    }
    digitalWrite(relayPin, HIGH);
  }
}
