/**
 * @file functions.h
 * @brief Function declarations for Wi-Fi handling, configuration, mDNS setup,
 * and web server management.
 *
 * This header file declares functions for loading configuration, handling Wi-Fi
 * station and AP modes, setting up mDNS, and managing the web server for the
 * ESP32 device.
 *
 * @note The functions defined here are intended for managing network
 * connections and serving files from LittleFS to create a web interface for the
 * ESP32.
 *
 * @version 1.0
 * @date 2024-10-28
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
 * @param onTime The time when the light should be turned on (in string format,
 * e.g., "08:00").
 * @param offTime The time when the light should be turned off (in string
 * format, e.g., "18:00").
 * @return true if the time settings are successfully saved, false otherwise.
 */
bool saveTimeSettings(const char *onTime, const char *offTime);

bool handleRTC();

/**
 * @brief Converts a time string in "hh:mm AM/PM" format (char*) to a DateTime
 * object based on the current date from the RTC.
 *
 * @param timeString The time string in the format "hh:mm AM/PM" as a char*.
 * @return DateTime The DateTime object representing the parsed time, or a
 * default DateTime (2000-01-01 00:00:00) if parsing fails.
 */
DateTime stringToDateTime(const char *timeString);

#endif // FUNCTIONS_H
