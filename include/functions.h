/**
 * @file functions.h
 * @brief Function declarations for Wi-Fi handling, configuration, mDNS setup,
 * RTC management, NTP synchronization, and web server management.
 *
 * This header file declares functions for loading configuration, handling Wi-Fi
 * station and AP modes, setting up mDNS, managing the web server for the
 * ESP32 device, and synchronizing time using RTC and NTP.
 *
 * @note The functions defined here are intended for managing network
 * connections, time synchronization, and serving files from LittleFS to create
 * a web interface for the ESP32.
 *
 * @version 0.1.0
 * @date 2024-12-6
 * @author WittyWizard
 */

#pragma once

#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "variables.h"
#include <ArduinoJson.h>
#include <LittleFS.h>

/**
 * @brief Loads the configuration from the file system.
 *
 * This function reads the configuration file stored in the file system and
 * parses it into a JsonDocument. It must be called during the setup process.
 *
 * @return JsonDocument containing the parsed configuration data.
 */
JsonDocument loadConfiguration();

/**
 * @brief Handles the connection to a Wi-Fi station.
 *
 * This function attempts to connect the ESP32 to a Wi-Fi network using the
 * provided SSID and password. If the connection is successful, it returns true.
 * Otherwise, it returns false.
 *
 * @param ssid A pointer to a character array to hold the Wi-Fi SSID.
 * @param ssid_n The size of the SSID character array.
 * @param password A pointer to a character array to hold the Wi-Fi password.
 * @param password_n The size of the password character array.
 * @param config The JsonDocument containing configuration data.
 *
 * @return true if the connection is successful, false otherwise.
 */
bool handleWiFiStation(char *ssid, size_t ssid_n, char *password,
                       size_t password_n, JsonDocument config);

/**
 * @brief Sets up the device as an Access Point (AP).
 *
 * If the device fails to connect to a Wi-Fi network, this function sets up an
 * access point with the provided SSID and password. It uses the configuration
 * data to determine AP settings.
 *
 * @param ssid A pointer to a character array to hold the AP SSID.
 * @param ssid_n The size of the SSID character array.
 * @param password A pointer to a character array to hold the AP password.
 * @param password_n The size of the password character array.
 * @param config The JsonDocument containing configuration data.
 */
void handleAP(char *ssid, size_t ssid_n, char *password, size_t password_n,
              JsonDocument config);

/**
 * @brief Initializes and configures mDNS for the device.
 *
 * This function sets up the mDNS responder with the desired hostname, allowing
 * the device to be accessed using a domain name (e.g., http://lightwave.local).
 * It should be called after the device is connected to a Wi-Fi network.
 */
void handleMDNS();

/**
 * @brief Sets up the web server and serves files from LittleFS.
 *
 * This function mounts the LittleFS file system, sets up routes for serving
 * HTML, CSS, and JavaScript files, and starts the web server. It should be
 * called in the setup function after the device is connected to Wi-Fi.
 *
 * The function handles requests for common file types and serves them using
 * the appropriate MIME types. It also sets up the server to handle any other
 * static files present in LittleFS.
 */
void handleWebServer();

/**
 * @brief Updates the SSID and password fields in the LittleFS configuration
 * file.
 *
 * This function modifies only the "ssid" and "password" fields of the existing
 * JSON configuration in the file and writes the updated values back to the
 * file.
 *
 * @param newSSID The new SSID to be updated in the configuration.
 * @param newPassword The new password to be updated in the configuration.
 * @return true if the update is successful, false otherwise.
 */
bool updateWiFiCredentials(const char *newSSID, const char *newPassword);

/**
 * @brief Saves the on and off time settings to the LittleFS configuration file.
 *
 * This function writes the provided "onTime" and "offTime" values to the
 * LittleFS file system. It creates or updates a JSON file that stores the time
 * settings. If the file cannot be opened or the settings cannot be saved, the
 * function returns false.
 *
 * @param onTime The time when the light should be turned on (unix epoch time).
 * @param offTime The time when the light should be turned off (unix epoch
 * time).
 * @return true if the time settings are successfully saved, false otherwise.
 */
bool saveTimeSettings(unsigned int onTime, unsigned int offTime);

/**
 * @brief Initializes and configures the RTC for the device.
 *
 * This function sets up the Real Time Clock (RTC) on the ESP32 device. It
 * should be called during the setup process to ensure the RTC is running
 * and has accurate time. If the RTC fails to initialize, the function
 * returns false.
 *
 * @return true if the RTC is successfully initialized, false otherwise.
 */
bool handleRTC();

/**
 * @brief Updates the RTC time using the NTP server.
 *
 * This function synchronizes the Real Time Clock (RTC) with the NTP server
 * time. It fetches the current time from the configured NTP server and sets the
 * RTC with the updated time. It should be called after Wi-Fi is connected.
 *
 * @return true if the RTC is successfully updated from the NTP server, false
 * otherwise.
 */
bool updateRTCFromNTP();

/**
 * @brief Blinks an error LED to indicate a failure in both NTP and RTC.
 *
 * This function continuously blinks an LED connected to `errorLedPin`
 * with a 500 ms on/off interval. It is called when both NTP and RTC
 * time synchronization fail, providing a visual indication of an error.
 *
 * @note This function runs in an infinite loop and halts further
 * execution of the program.
 */
void blinkErrorLed();

/**
 * @brief Handles JSON data received in an HTTP POST request.
 *
 * This function accumulates chunks of incoming JSON data, deserializes it, and
 * processes it using a user-defined callback function. It also handles errors
 * in JSON parsing and responds with appropriate status codes if the data is
 * invalid.
 *
 * This function is useful when handling HTTP POST requests that send JSON data
 * to the ESP32, such as configuration or command requests from a client
 * interface.
 *
 * @param request Pointer to the AsyncWebServerRequest object representing the
 * HTTP request.
 * @param data Pointer to the incoming data chunk received in the request.
 * @param len Length of the current data chunk.
 * @param index Current index of the data chunk in the full data stream.
 * @param total Total expected size of the complete data stream.
 * @param processData A callback function that takes a reference to a
 * JsonDocument and a pointer to AsyncWebServerRequest. This function is called
 * to process the JSON data and respond to the client after successful
 * deserialization.
 *
 */
void handleJsonRequest(
    AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index,
    size_t total,
    std::function<void(AsyncWebServerRequest *, JsonDocument &)> processData);

#endif // FUNCTIONS_H
