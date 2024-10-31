#include "variables.h"

AsyncWebServer server(80);
RTC_DS3231 rtc;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 19800);

bool rtcFailed = false;
bool ntpFailed = false;
DateTime turnOff;
DateTime turnOn;

bool isOn = false;
bool validOnOffTimes = false;

const int errorLedPin = 10;
const int relayPin = 9;