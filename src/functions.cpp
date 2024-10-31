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

  // Successful connection
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

  server.on("css/style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/style.css", "text/css");
  });

  server.on("js/script.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/script.js", "application/javascript");
  });

  server.on(
      "/connect", HTTP_POST, [](AsyncWebServerRequest *request) {}, nullptr,
      [](AsyncWebServerRequest *request, uint8_t *data, size_t len,
         size_t index, size_t total) {
        String body;

        for (size_t i = 0; i < len; i++) {
          body += (char)data[i];
        }

        if (index + len == total) {
          JsonDocument doc;

          DeserializationError error = deserializeJson(doc, body);

          if (error) {
            Serial.print("JSON parsing failed: ");
            Serial.println(error.c_str());
            request->send(400, "text/plain", "Invalid JSON");
            return;
          }

          String ssid = doc["ssid"] | "";
          String password = doc["password"] | "";

          if (ssid != "" && password != "") {
            Serial.printf("Received SSID: %s, Password: %s\n", ssid.c_str(),
                          password.c_str());

            if (updateWiFiCredentials(ssid.c_str(), password.c_str())) {
              request->send(200, "text/plain",
                            "Wi-Fi credentials received and saved. Attempting "
                            "to connect...");
              delay(500);
              ESP.restart();
            } else {
              request->send(500, "text/plain",
                            "Failed to save Wi-Fi credentials.");
            }

          } else {
            request->send(400, "text/plain", "Missing SSID or Password");
          }
        }
      });

  server.on(
      "/setup", HTTP_POST, [](AsyncWebServerRequest *request) {}, nullptr,
      [](AsyncWebServerRequest *request, uint8_t *data, size_t len,
         size_t index, size_t total) {
        // Static variable to accumulate the incoming body data
        static String body;

        // Accumulate data chunks into the body string
        body += String((char *)data).substring(0, len);
        // If the entire body is received
        if (index + len == total) {
          // Create a JSON document to parse the body
          JsonDocument doc;

          // Parse the JSON payload
          DeserializationError error = deserializeJson(doc, body);

          // Clear the body string for the next request
          body = "";

          // Check for JSON parsing errors
          if (error) {
            Serial.print("JSON parsing failed: ");
            Serial.println(error.c_str());
            request->send(400, "text/plain", "Invalid JSON");
            return;
          }

          // Extract onTime and offTime from the JSON document
          int onTime = doc["onTime"] | 0;
          int offTime = doc["offTime"] | 0;

          // Check if both onTime and offTime are present
          if (onTime != 0 && offTime != 0) {
            Serial.printf("Received onTime: %i, offTime: %i\n", onTime,
                          offTime);

            // Save the time settings in LittleFS
            if (saveTimeSettings(onTime, offTime)) {
              request->send(200, "text/plain",
                            "Time settings received and saved successfully.");
            } else {
              request->send(500, "text/plain", "Failed to save time settings.");
            }

          } else {
            request->send(400, "text/plain", "Missing onTime or offTime");
          }
        }
      });

  server.on(
      "/setTime", HTTP_POST, [](AsyncWebServerRequest *request) {}, nullptr,
      [](AsyncWebServerRequest *request, uint8_t *data, size_t len,
         size_t index, size_t total) {
        // Static variable to accumulate the incoming body data
        static String body;

        // Accumulate data chunks into the body string
        body += String((char *)data).substring(0, len);

        // If the entire body is received
        if (index + len == total) {
          // Create a JSON document to parse the body
          JsonDocument doc;

          // Parse the JSON payload
          DeserializationError error = deserializeJson(doc, body);

          // Clear the body string for the next request
          body = "";

          // Check for JSON parsing errors
          if (error) {
            Serial.print("JSON parsing failed: ");
            Serial.println(error.c_str());
            request->send(400, "text/plain", "Invalid JSON");
            return;
          }

          // Extract onTime and offTime from the JSON document
          unsigned int currentTime = doc["currentTime"] | 0;
          Serial.println(currentTime);

          // Check if both onTime and offTime are present
          if (currentTime != 0) {
            DateTime parsedTime = DateTime(currentTime);
            rtc.adjust(parsedTime);
            request->send(200, "text/plain",
                          "Time settings received and saved successfully.");
          } else {
            request->send(400, "text/plain", "Missing Current Time");
          }
        }
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
  // Initialize LittleFS if not already mounted
  if (!LittleFS.begin()) {
    Serial.println(
        "An error has occurred while mounting or formatting LittleFS");
    return false;
  }

  // Create a JSON document to store the time settings
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

  file = LittleFS.open("/config.json", "w");
  if (!file) {
    Serial.println("Failed to open time settings file for writing");
    return false;
  }

  // Serialize the JSON document and write it to the file
  if (serializeJson(doc, file) == 0) {
    Serial.println("Failed to write time settings to file");
    file.close();
    return false;
  }

  // Close the file
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
  // Start the NTP client
  timeClient.begin();

  // Attempt to get the time from the NTP server
  if (!timeClient.update()) {
    Serial.println("Failed to get time from NTP server");
    return false;
  }

  // Get the current time as epoch time
  unsigned long epochTime = timeClient.getEpochTime();

  // Convert the epoch time to a DateTime object
  DateTime ntpTime = DateTime(epochTime);

  // Update the RTC with the new time
  rtc.adjust(ntpTime);
  return true;
}