#include "TimeManager.h"

// Default NTP server, GMT+0 offset, update interval 60 seconds
const long gmtOffset_sec = 0;
const int daylightOffset_sec = 3600;

TimeManager::TimeManager() {
    // Initialize the pointer to null. It will be created in begin().
    timeClient = nullptr;
}

void TimeManager::begin() {
    // Now that UDP is available, create the NTPClient instance
    timeClient = new NTPClient(ntpUDP, "pool.ntp.org", gmtOffset_sec, 60000);
    timeClient->begin();
}

void TimeManager::loop() {
    // The update() method should be called regularly to sync the time
    if (timeClient != nullptr) {
        timeClient->update();
    }
}

String TimeManager::getFormattedTime() const {
    if (timeClient != nullptr) {
        return timeClient->getFormattedTime();
    }
    return "Time not available";
}

time_t TimeManager::getEpochTime() const {
    if (timeClient != nullptr) {
        return timeClient->getEpochTime();
    }
    return 0;
}
