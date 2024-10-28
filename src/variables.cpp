#include "variables.h"

AsyncWebServer server(80);
RTC_DS3231 rtc;

bool rtcFailed = false;