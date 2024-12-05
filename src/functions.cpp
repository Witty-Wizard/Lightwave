#include "functions.h"
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <RTClib.h>
#include <SPI.h>
#include <WiFi.h>

JsonDocument loadConfiguration() {
  if (!LittleFS.begin()) {

    Serial.println(
        "An error has occurred while mounting or formatting LittleFS");
    return JsonDocument();
  }
  JsonDocument doc;
  File file = LittleFS.open("/config.json", "r");
  if (!file) {
    Serial.println("Failed to open configuration file");
    return doc;
  }

  DeserializationError error = deserializeJson(doc, file);
  if (error) {
    Serial.print("Failed to parse configuration file: ");
    Serial.println(error.f_str());
    file.close();
    return doc;
  }

  if (doc.isNull()) {
    Serial.println("Failed to load configuration");
    return JsonDocument();
  }

  file.close();
  return doc;
}

bool handleWiFiStation(char *ssid, size_t ssid_n, char *password,
                       size_t password_n, JsonDocument config) {

  strlcpy(ssid, config["ssid"] | "", ssid_n);
  strlcpy(password, config["password"] | "", password_n);

  Serial.print("Connecting to WiFi SSID: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  unsigned long startAttemptTime = millis();
  const unsigned long connectionTimeout = 10000;

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);

    if (millis() - startAttemptTime >= connectionTimeout) {
      Serial.println("\nConnection Timeout: Failed to connect to WiFi.");
      return false;
    }
  }

  Serial.println("\nConnected to WiFi!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  return (WiFi.status() == WL_CONNECTED);
}

void handleAP(char *ssid, size_t ssid_n, char *password, size_t password_n,
              JsonDocument config) {
  strlcpy(ssid, config["ssidAP"] | "", ssid_n);
  strlcpy(password, config["passwordAP"] | "", password_n);

  Serial.println("Setting up Access Point...");
  Serial.print("AP SSID: ");
  Serial.println(ssid);
  Serial.print("AP Password: ");
  Serial.println(password);

  if (WiFi.softAP(ssid, password)) {
    Serial.println("Access Point started successfully!");
    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());
  } else {
    Serial.println("Failed to start Access Point.");
  }
}

void handleMDNS() {
  if (!MDNS.begin("lightwave")) {
    Serial.println("Error setting up mDNS responder!");
  } else {
    Serial.println("mDNS responder started");
  }
}

void handleWebServer() {
  if (!LittleFS.begin()) {
    Serial.println("An error has occurred while mounting LittleFS");
    return;
  }
  Serial.println("LittleFS mounted successfully");

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/index.html", "text/html");
  });

  server.on(
      "/api/connect", HTTP_POST, [](AsyncWebServerRequest *request) {}, nullptr,
      [](AsyncWebServerRequest *request, uint8_t *data, size_t len,
         size_t index, size_t total) {
        handleJsonRequest(
            request, data, len, index, total,
            [](AsyncWebServerRequest *request, JsonDocument &doc) {
              String ssid = doc["ssid"] | "";
              String password = doc["password"] | "";

              if (ssid != "" && password != "") {
                Serial.printf("Received SSID: %s, Password: %s\n", ssid.c_str(),
                              password.c_str());

                if (updateWiFiCredentials(ssid.c_str(), password.c_str())) {
                  request->send(200, "text/plain",
                                "Wi-Fi credentials received and saved. "
                                "Attempting to connect...");
                  delay(500);
                  ESP.restart();
                } else {
                  request->send(500, "text/plain",
                                "Failed to save Wi-Fi credentials.");
                }
              } else {
                request->send(400, "text/plain", "Missing SSID or Password");
              }
            });
      });

  server.on(
      "/api/setup", HTTP_POST, [](AsyncWebServerRequest *request) {}, nullptr,
      [](AsyncWebServerRequest *request, uint8_t *data, size_t len,
         size_t index, size_t total) {
        handleJsonRequest(
            request, data, len, index, total,
            [](AsyncWebServerRequest *request, JsonDocument &doc) {
              int onTime = doc["onTime"] | 0;
              int offTime = doc["offTime"] | 0;

              if (onTime != 0 && offTime != 0) {
                DateTime onTimeParse = DateTime(onTime);
                DateTime offTimeParse = DateTime(offTime);
                Serial.printf("Received onTime: %i:%i, offTime: %i:%i \n",
                              onTimeParse.hour(), onTimeParse.minute(),
                              offTimeParse.hour(), offTimeParse.minute());

                if (saveTimeSettings(onTime, offTime)) {
                  request->send(
                      200, "text/plain",
                      "Time settings received and saved successfully.");
                } else {
                  request->send(500, "text/plain",
                                "Failed to save time settings.");
                }
              } else {
                request->send(400, "text/plain", "Missing onTime or offTime");
              }
            });
      });

  server.on(
      "/api/setTime", HTTP_POST, [](AsyncWebServerRequest *request) {}, nullptr,
      [](AsyncWebServerRequest *request, uint8_t *data, size_t len,
         size_t index, size_t total) {
        handleJsonRequest(
            request, data, len, index, total,
            [](AsyncWebServerRequest *request, JsonDocument &doc) {
              unsigned int currentTime = doc["currentTime"] | 0;

              if (currentTime != 0) {
                DateTime parsedTime = DateTime(currentTime);
                rtc.adjust(parsedTime);
                request->send(200, "text/plain",
                              "Time settings received and saved successfully.");
              } else {
                request->send(400, "text/plain", "Missing Current Time");
              }
            });
      });

  server.on("/api/toggle", HTTP_GET, [](AsyncWebServerRequest *request) {
    isOn = !isOn;

    String response = "{\"isOn\": " + String(isOn ? "true" : "false") + "}";
    request->send(200, "application/json", response);

    Serial.println("State toggled: " + String(isOn ? "On" : "Off"));
  });

  server.on("/toggleGet", HTTP_GET, [](AsyncWebServerRequest *request) {
    String response = "{\"isOn\": " + String(isOn ? "true" : "false") + "}";
    request->send(200, "application/json", response);
  });

  server.serveStatic("/", LittleFS, "/");
  server.begin();
  Serial.println("Web server started");
}

bool updateWiFiCredentials(const char *newSSID, const char *newPassword) {
  if (!LittleFS.begin()) {
    Serial.println(
        "An error has occurred while mounting or formatting LittleFS");
    return false;
  }

  File file = LittleFS.open("/config.json", "r");
  if (!file) {
    Serial.println("Failed to open configuration file for reading");
    return false;
  }

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, file);

  file.close();

  if (error) {
    Serial.print("Failed to parse configuration file: ");
    Serial.println(error.c_str());
    return false;
  }

  doc["ssid"] = newSSID;
  doc["password"] = newPassword;

  file = LittleFS.open("/config.json", "w");
  if (!file) {
    Serial.println("Failed to open configuration file for writing");
    return false;
  }

  if (serializeJson(doc, file) == 0) {
    Serial.println("Failed to write updated configuration to file");
    file.close();
    return false;
  }

  file.close();
  Serial.println("Wi-Fi credentials updated successfully in /config.json");
  return true;
}

bool saveTimeSettings(unsigned int onTime, unsigned int offTime) {
  if (!LittleFS.begin()) {
    Serial.println(
        "An error has occurred while mounting or formatting LittleFS");
    return false;
  }

  JsonDocument doc;
  File file = LittleFS.open("/config.json", "r+");
  if (!file) {
    Serial.println("Failed to open time settings file for writing");
    return false;
  }

  DeserializationError error = deserializeJson(doc, file);
  if (error) {
    Serial.print("Failed to parse configuration file: ");
    Serial.println(error.f_str());
    file.close();
    return false;
  }
  if (doc.isNull()) {
    Serial.println("Failed to load configuration");
    return false;
  }
  doc["onTime"] = onTime;
  doc["offTime"] = offTime;

  turnOn = DateTime(onTime);
  turnOff = DateTime(offTime);
  validOnOffTimes = true;

  file = LittleFS.open("/config.json", "w");
  if (!file) {
    Serial.println("Failed to open time settings file for writing");
    return false;
  }

  if (serializeJson(doc, file) == 0) {
    Serial.println("Failed to write time settings to file");
    file.close();
    return false;
  }

  file.close();
  Serial.println("Time settings saved successfully to /config.json");
  return true;
}

bool handleRTC() {
  Wire.setPins(23, 18);
  if (!rtc.begin()) {
    Serial.println("RTC initialization failed!");
    return false;
  }

  Serial.println("RTC initialized successfully.");
  return true;
}

bool updateRTCFromNTP() {
  timeClient.begin();

  if (!timeClient.update()) {
    Serial.println("Failed to get time from NTP server");
    return false;
  }

  unsigned long epochTime = timeClient.getEpochTime();

  DateTime ntpTime = DateTime(epochTime);

  rtc.adjust(ntpTime);
  return true;
}

void blinkErrorLed() {
  while (true) {
    digitalWrite(errorLedPin, HIGH);
    delay(500);
    digitalWrite(errorLedPin, LOW);
    delay(500);
  }
}

void handleJsonRequest(
    AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index,
    size_t total,
    std::function<void(AsyncWebServerRequest *, JsonDocument &)> processData) {
  static String body;
  body += String((char *)data).substring(0, len);

  if (index + len == total) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, body);
    body = "";

    if (error) {
      Serial.print("JSON parsing failed: ");
      Serial.println(error.c_str());
      request->send(400, "text/plain", "Invalid JSON");
      return;
    }

    processData(request, doc);
  }
}