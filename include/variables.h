/**
 * @file variables.h
 * @brief External variable declarations for ESP32 project, including web
 * server, RTC, NTP client, and GPIO pins for relay and error LED.
 *
 * This header file declares external variables used across multiple modules
 * in the project, such as the web server, real-time clock, and network time
 * synchronization functionalities. It includes GPIO pin configurations, state
 * flags, and scheduling objects, enabling centralized handling of the ESP32's
 * on/off state, time-based scheduling, and error management.
 *
 * @version 0.1.0
 * @date 2024-11-01
 * @author WittyWizard
 */

#pragma once

#ifndef VARIABLES_H
#define VARIABLES_H

#include <ESPAsyncWebServer.h> ///< Library for handling asynchronous web server on ESP32.
#include <NTPClient.h> ///< Library for network time protocol (NTP) synchronization.
#include <RTClib.h> ///< Library for interfacing with DS3231 real-time clock (RTC).
#include <SPI.h>    ///< SPI library for communication with peripherals.
#include <WiFiUdp.h> ///< WiFiUdp library for UDP communication on ESP32.

/**
 * @brief Asynchronous web server instance for managing HTTP requests on ESP32.
 *
 * This server handles client requests and serves files from the ESP32's
 * LittleFS file system, enabling remote configuration and monitoring of
 * the device.
 */
extern AsyncWebServer server;

/**
 * @brief Real-Time Clock (RTC) object using the DS3231 RTC module.
 *
 * This object is used to keep track of date and time independently of
 * the ESP32's power state, allowing for accurate timekeeping and
 * scheduled actions even across device restarts.
 */
extern RTC_DS3231 rtc;

/**
 * @brief UDP object for NTP client communication.
 *
 * The ntpUDP object is essential for creating the UDP connection
 * required for NTP time synchronization, enabling accurate time updates
 * via the network.
 */
extern WiFiUDP ntpUDP;

/**
 * @brief NTP client instance for managing network time synchronization.
 *
 * The NTP client connects to an NTP server to retrieve the current time
 * and ensures that the ESP32's clock is synchronized with the network
 * time, providing a reliable time reference for the device.
 */
extern NTPClient timeClient;

/**
 * @brief Flag indicating if the RTC module initialization failed.
 *
 * This boolean is set to true if the RTC fails to initialize or is not
 * detected, allowing the system to fall back to alternative timekeeping
 * methods if necessary.
 */
extern bool rtcFailed;

/**
 * @brief Flag indicating if the NTP time synchronization failed.
 *
 * This boolean is set to true if the NTP client fails to retrieve the
 * current time from the network, indicating network connectivity or
 * server issues.
 */
extern bool ntpFailed;

/**
 * @brief DateTime object representing the scheduled turn-off time.
 *
 * This object holds the date and time at which the device is scheduled
 * to turn off, allowing for automated control based on the user-defined
 * schedule.
 */
extern DateTime turnOff;

/**
 * @brief DateTime object representing the scheduled turn-on time.
 *
 * This object holds the date and time at which the device is scheduled
 * to turn on, enabling scheduled operation according to user preferences.
 */
extern DateTime turnOn;

/**
 * @brief Boolean indicating the current on/off state of the device.
 *
 * This flag represents the device’s operational state, where true
 * signifies that the device is currently active, and false indicates
 * it is turned off.
 */
extern bool isOn;

/**
 * @brief Flag indicating if the on/off scheduling times are valid.
 *
 * This flag is set to true if the turn-on and turn-off times are valid
 * and can be used for scheduling. It helps prevent erroneous actions
 * due to invalid or uninitialized scheduling times.
 */
extern bool validOnOffTimes;

/**
 * @brief GPIO pin number connected to the error LED indicator.
 *
 * This constant represents the GPIO pin to which the error LED is
 * connected, allowing for visual error signaling to users in case
 * of issues like RTC or NTP failures.
 */
extern const int errorLedPin;

/**
 * @brief GPIO pin number connected to the relay for controlling the device.
 *
 * This constant specifies the GPIO pin connected to the relay, which is
 * used to control the device's power state by turning it on or off
 * based on the specified schedule.
 */
extern const int relayPin;

#endif // VARIABLES_H
