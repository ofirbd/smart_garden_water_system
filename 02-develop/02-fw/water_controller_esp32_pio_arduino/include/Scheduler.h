/**
 * @file Scheduler.h
 * @brief Manages the watering schedule.
 * @author ofirbd
 * @version 1.0
 * @date 2025-07-04
 */
#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "TimeManager.h"
#include "ValveController.h"

// Define the structure for a single watering action
struct WateringAction {
    int valve;
    int startHour;
    int startMinute;
    int durationMinutes;
};

// Define the structure for a daily program
struct Program {
    bool enabled;
    // Array of 7 booleans, Sunday=0 to Saturday=6
    bool days[7]; 
    // We'll support a fixed number of actions per program for simplicity
    WateringAction actions[5];
    int actionCount;
};

class MqttClient; // Forward declaration

class Scheduler {
public:
    public:
    Scheduler(TimeManager* timeManager, ValveController* valveController);
    void begin();
    void loop();
    bool updateSchedule(const char* jsonPayload);
    String getScheduleJson() const;
    void setMqttClient(MqttClient* mqttClient);
    void publishSchedule() const;

    void loadSchedule();

private:
    bool _parseAndLoad(const char* jsonPayload);
    void _saveScheduleToSPIFFS();

    TimeManager* _timeManager;
    ValveController* _valveController;
    MqttClient* _mqttClient; // MQTT client for publishing status

    Program _programs[5]; // Support up to 5 different programs
    int _programCount;

    // State tracking for active watering
    struct ActiveWatering {
        bool active;
        int valve;
        time_t stopTime;
    } _activeWatering[3]; // One for each valve

    static int dayIndex(String day); // Declare as static member function
};

#endif // SCHEDULER_H
