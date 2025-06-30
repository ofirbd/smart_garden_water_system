#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H

#include <NTPClient.h>
#include <WiFiUdp.h>

class TimeManager {
public:
    TimeManager();
    void begin();
    void loop();
    String getFormattedTime() const;
    time_t getEpochTime() const;

private:
    WiFiUDP ntpUDP;
    NTPClient* timeClient; // Use a pointer to allow for constructor initialization
};

#endif // TIME_MANAGER_H
