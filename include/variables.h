#pragma once

#ifndef VARIABLES_H
#define VARIABLES_H
#include <ESPAsyncWebServer.h>
#include <RTClib.h>
#include <SPI.h>

extern AsyncWebServer server;
extern RTC_DS3231 rtc;

extern bool rtcFailed;

#endif