#pragma once

#ifndef VARIABLES_H
#define VARIABLES_H
#include <ESPAsyncWebServer.h>
#include <NTPClient.h>
#include <RTClib.h>
#include <SPI.h>
#include <WiFiUdp.h>

extern AsyncWebServer server;
extern RTC_DS3231 rtc;
extern WiFiUDP ntpUDP;
extern NTPClient timeClient;

extern bool rtcFailed;
extern bool ntpFailed;

extern DateTime turnOn;
extern DateTime turnOff;

#endif