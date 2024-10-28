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

  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/style.css", "text/css");
  });

  server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request) {
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
          String onTime = doc["onTime"] | "";
          String offTime = doc["offTime"] | "";

          // Check if both onTime and offTime are present
          if (onTime != "" && offTime != "") {
            Serial.printf("Received onTime: %s, offTime: %s\n", onTime.c_str(),
                          offTime.c_str());

            // Save the time settings in LittleFS
            if (saveTimeSettings(onTime.c_str(), offTime.c_str())) {
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
          String currentTime = doc["currentTime"] | "";

          // Check if both onTime and offTime are present
          if (currentTime != "") {
            DateTime parsedTime = stringToDateTime(currentTime.c_str());
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

bool saveTimeSettings(const char *onTime, const char *offTime) {
  // Initialize LittleFS if not already mounted
  if (!LittleFS.begin()) {
    Serial.println(
        "An error has occurred while mounting or formatting LittleFS");
    return false;
  }

  // Create a JSON document to store the time settings
  JsonDocument doc;
  doc["onTime"] = onTime;
  doc["offTime"] = offTime;

  // Open the configuration file for writing (overwrite mode)
  File file = LittleFS.open("/time.json", "w");
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
  Serial.println("Time settings saved successfully to /time.json");
  ESP.restart();
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

DateTime stringToDateTime(const char *timeString) {
  // Check if the timeString length is valid (8 or 9 characters: "hh:mm AM/PM")
  size_t len = strlen(timeString);
  if (len < 8 || len > 9 || timeString[2] != ':' ||
      (strstr(timeString, "AM") == nullptr &&
       strstr(timeString, "PM") == nullptr)) {
    Serial.println("Invalid time format");
    return DateTime(2000, 1, 1, 0, 0, 0); // Return a default DateTime on error
  }

  // Extract the hour, minute, and period (AM/PM) components
  int hour = (timeString[0] - '0') * 10 + (timeString[1] - '0');
  int minute = (timeString[3] - '0') * 10 + (timeString[4] - '0');
  char period[3] = {timeString[len - 2], timeString[len - 1], '\0'};

  // Validate the hour, minute, and period
  if (hour < 1 || hour > 12 || minute < 0 || minute > 59 ||
      (strcmp(period, "AM") != 0 && strcmp(period, "PM") != 0)) {
    Serial.println("Invalid hour, minute, or period value");
    return DateTime(2000, 1, 1, 0, 0, 0); // Return a default DateTime on error
  }

  // Convert hour to 24-hour format
  if (strcmp(period, "PM") == 0 && hour != 12) {
    hour += 12;
  } else if (strcmp(period, "AM") == 0 && hour == 12) {
    hour = 0;
  }

  // Get the current date from the RTC
  DateTime now = rtc.now();

  // Create a new DateTime object with the parsed time and current date
  return DateTime(now.year(), now.month(), now.day(), hour, minute, 0);
}