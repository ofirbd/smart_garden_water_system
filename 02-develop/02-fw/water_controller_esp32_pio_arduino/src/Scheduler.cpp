/**
 * @file Scheduler.cpp
 * @brief Implements the Scheduler class for managing watering schedules.
 * 
 * This file contains the implementation of the Scheduler class, which is responsible
 * for managing the watering schedule. It loads the schedule from SPIFFS on startup,
 * saves new schedules received via MQTT, and checks the current time to trigger
 * watering events. The use of a separate class for scheduling follows the single-
 * responsibility principle, making the code more modular and easier to maintain.
 * The schedule is stored in a structured format, and the class provides methods
 * for updating and retrieving the schedule in JSON format.
 * 
 * @author ofirbd
 * @version 1.0
 * @date 2025-07-04
 */
#include <SPIFFS.h>
#include "MqttClient.h"
#include "Scheduler.h"
#include "Config.h"

#define SCHEDULE_FILE "/schedule.json"

Scheduler::Scheduler(TimeManager* timeManager, ValveController* valveController)
: _timeManager(timeManager), _valveController(valveController), _mqttClient(nullptr), _programCount(0) {
    // Initialize active watering states
    for (int i = 0; i < 3; ++i) {
        _activeWatering[i] = {false, 0, 0};
    }
}

void Scheduler::begin() {
    if (!SPIFFS.begin(true)) {
        Serial.println("An Error has occurred while mounting SPIFFS");
        return;
    }
    loadSchedule();
}

void Scheduler::loop() {
    time_t now = _timeManager->getEpochTime();
    if (now == 0) return; // Time not synced yet

    // Check for starting events
    for (int i = 0; i < _programCount; ++i) {
        Program& p = _programs[i];
        if (!p.enabled) continue;

        // tm_wday: Sunday=0, ..., Saturday=6
        int currentDay = localtime(&now)->tm_wday;
        if (!p.days[currentDay]) continue;

        for (int j = 0; j < p.actionCount; ++j) {
            WateringAction& action = p.actions[j];
            if (localtime(&now)->tm_hour == action.startHour && localtime(&now)->tm_min == action.startMinute) {
                // Check if this valve is already watering
                if (!_activeWatering[action.valve - 1].active) {
                    Serial.printf("Scheduler starting valve %d for %d minutes.\n", action.valve, action.durationMinutes);
                    _valveController->openValve(action.valve);
                    _activeWatering[action.valve - 1] = {true, action.valve, now + action.durationMinutes * 60};
                }
            }
        }
    }

    // Check for stopping events
    for (int i = 0; i < 3; ++i) {
        if (_activeWatering[i].active && now >= _activeWatering[i].stopTime) {
            Serial.printf("Scheduler stopping valve %d.\n", _activeWatering[i].valve);
            _valveController->closeValve(_activeWatering[i].valve);
            _activeWatering[i] = {false, 0, 0}; // Reset state
        }
    }
}

void Scheduler::setMqttClient(MqttClient* mqttClient) {
    _mqttClient = mqttClient;
}

bool Scheduler::updateSchedule(const char* jsonPayload) {
    Serial.print("Scheduler::updateSchedule() received payload: [");
    Serial.print(jsonPayload);
    Serial.println("]");

    if (_parseAndLoad(jsonPayload)) {
        // Save the new schedule to SPIFFS
        _saveScheduleToSPIFFS();
        // Publish the new state to MQTT
        publishSchedule();
        return true;
    }
    return false;
}

void Scheduler::publishSchedule() const {
    if (!_mqttClient || !_mqttClient->connected()) {
        Serial.println("MQTT client not available or not connected. Cannot publish schedule.");
        return;
    }
    String scheduleJson = getScheduleJson();
    _mqttClient->publish(MQTT_TOPIC_SCHEDULE_STATE, scheduleJson.c_str(), false); // do not Retain the message
    Serial.println("Published schedule to MQTT topic: schedule/state");
}

void Scheduler::loadSchedule() {
    if (SPIFFS.exists(SCHEDULE_FILE)) {
        File file = SPIFFS.open(SCHEDULE_FILE, "r");
        if (file) {
            Serial.println("Reading existing schedule from SPIFFS...");
            String content = file.readString();
            file.close();
            _parseAndLoad(content.c_str()); // Load, but don't save here
        } else {
             Serial.println("Failed to open schedule file for reading.");
        }
    } else {
        Serial.println("No schedule file found, using default empty schedule.");
    }
}

String Scheduler::getScheduleJson() const {
    DynamicJsonDocument doc(2048);

    // Top-level object
    doc["enabled"] = true; // Assuming the schedule system is always enabled

    JsonArray programs = doc.createNestedArray("programs");

    for (int i = 0; i < _programCount; ++i) {
        JsonObject program_json = programs.createNestedObject();
        program_json["enabled"] = _programs[i].enabled;

        JsonArray days = program_json.createNestedArray("days");
        const char* dayNames[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
        for(int d = 0; d < 7; ++d) {
            if (_programs[i].days[d]) {
                days.add(dayNames[d]);
            }
        }

        JsonArray actions = program_json.createNestedArray("actions");
        for (int j = 0; j < _programs[i].actionCount; ++j) {
            JsonObject action_json = actions.createNestedObject();
            action_json["valve"] = _programs[i].actions[j].valve;
            char time_str[6];
            sprintf(time_str, "%02d:%02d", _programs[i].actions[j].startHour, _programs[i].actions[j].startMinute);
            action_json["time"] = time_str;
            action_json["duration_minutes"] = _programs[i].actions[j].durationMinutes;
        }
    }

    String jsonString;
    serializeJson(doc, jsonString);
    return jsonString;
}

void Scheduler::_saveScheduleToSPIFFS() {
    String jsonString = getScheduleJson();
    File file = SPIFFS.open(SCHEDULE_FILE, "w");
    if (!file) {
        Serial.println("Failed to open schedule file for writing");
        return;
    }
    
    if (file.print(jsonString) == 0) { // Use print() for String
        Serial.println(F("Failed to write to schedule file"));
    } else {
        Serial.println(F("Schedule saved successfully to SPIFFS"));
    }
    file.close();
}

// A helper to convert day string to index (Sunday=0)
int Scheduler::dayIndex(String day) {
    if (day == "Sunday") return 0;
    if (day == "Monday") return 1;
    if (day == "Tuesday") return 2;
    if (day == "Wednesday") return 3;
    if (day == "Thursday") return 4;
    if (day == "Friday") return 5;
    if (day == "Saturday") return 6;
    return -1;
}

bool Scheduler::_parseAndLoad(const char* jsonPayload) {
    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, jsonPayload);

    if (error) {
        Serial.print(F("_parseAndLoad() deserializeJson() failed: "));
        Serial.println(error.c_str());
        return false; // Parsing failed
    }

    _programCount = 0;
    JsonArray programs = doc["programs"].as<JsonArray>();
    for (JsonObject program_json : programs) {
        if (_programCount >= 5) break;
        Program& p = _programs[_programCount];
        p.enabled = program_json["enabled"].as<bool>() | true;
        
        // Reset days
        for(int i=0; i<7; ++i) p.days[i] = false;
        for (JsonVariant day : program_json["days"].as<JsonArray>()) {
            int idx = dayIndex(day.as<String>());
            if (idx != -1) p.days[idx] = true;
        }

        p.actionCount = 0;
        JsonArray actions = program_json["actions"].as<JsonArray>();
        for (JsonObject action_json : actions) {
            if (p.actionCount >= 5) break;
            WateringAction& a = p.actions[p.actionCount];
            a.valve = action_json["valve"];
            String time = action_json["time"]; // "08:00"
            a.startHour = time.substring(0, 2).toInt();
            a.startMinute = time.substring(3, 5).toInt();
            a.durationMinutes = action_json["duration_minutes"];
            p.actionCount++;
        }
        _programCount++;
    }
    Serial.printf("Successfully parsed and loaded %d programs.\n", _programCount);
    return true;
}

