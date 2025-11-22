/**
 * @file MqttClient.cpp
 * @brief Implements the MqttClient class for handling MQTT communication.
 * 
 * This file contains the implementation of the MqttClient class, which is responsible
 * for all MQTT-related functionality. It encapsulates the PubSubClient library to
 * provide a clean interface for connecting, publishing, and subscribing to topics.
 * Best practices are followed by using a static callback function to handle incoming
 * messages, which then calls a class instance method. This allows the callback to
 * be used with the C-style function pointer required by the library while still
 * maintaining an object-oriented structure. The class also handles automatic
 * reconnection to the broker, ensuring a resilient connection.
 * 
 * @author ofirbd
 * @version 1.0
 * @date 2025-07-04
 */
#include "MqttClient.h"

MqttClient* MqttClient::_instance = nullptr; // Initialize static instance pointer

MqttClient::MqttClient(Scheduler* scheduler, ValveController* valveController)
: _client(_espClient), _scheduler(scheduler), _valveController(valveController) {
    _instance = this; // Set the static instance pointer
}

void MqttClient::begin() {
    _client.setBufferSize(512); // was 512
    _client.setServer(MQTT_BROKER_IP, MQTT_PORT);
    _client.setCallback(MqttClient::staticCallback);
}

void MqttClient::loop() {
    if (!_client.connected()) {
        long now = millis();
        if (now - lastReconnectAttempt > 5000) { // Try to reconnect every 5 seconds
            lastReconnectAttempt = now;
            Serial.println("MQTT: Attempting reconnect...");
            reconnect();
        }
    } else {
        _client.loop();
        // Serial.println("MQTT: Client loop running."); // Too verbose, enable if needed
    }
}

bool MqttClient::publish(const char* topic, const char* payload, bool retained) {
    return _client.publish(topic, payload, retained);
}

bool MqttClient::publish(const char* topic, const uint8_t* payload, unsigned int plength, bool retained) {
    return _client.publish(topic, payload, plength, retained);
}

bool MqttClient::subscribe(const char* topic, uint8_t qos) {
    return _client.subscribe(topic, qos);
}

bool MqttClient::connected() {
    return _client.connected();
}

void MqttClient::reconnect() {
    Serial.print("Attempting MQTT connection...");
    if (_client.connect(MQTT_CLIENT_ID, MQTT_TOPIC_CONTROLLER_STATUS, 1, true, "offline")) {
        Serial.println("connected");
        _client.publish(MQTT_TOPIC_CONTROLLER_STATUS, "online", true); // Retain the online status
        subscribe(MQTT_TOPIC_VALVE_COMMAND);
        if (subscribe(MQTT_TOPIC_SCHEDULE_SET, 1)) {
            Serial.println("Subscribed to schedule topic successfully.");
        } else {
            Serial.print("Failed to subscribe to schedule topic. Error: ");
            Serial.println(_client.state());
        }
        if (subscribe(MQTT_TOPIC_SCHEDULE_GET, 1)) {
            Serial.println("Subscribed to schedule GET topic successfully.");
        } else {
            Serial.print("Failed to subscribe to schedule GET topic. Error: ");
            Serial.println(_client.state());
        }
        // Process any retained messages immediately after subscribing
        _client.loop();
        delay(100); // Give a small delay for the message to be processed

        // Publish the current schedule from the device to ensure it's retained on the broker
        _scheduler->publishSchedule();
        Serial.println("Published current schedule to broker.");
    } else {
        Serial.print("failed, rc=");
        Serial.print(_client.state());
        Serial.println(" try again in 5 seconds");
    }
}

void MqttClient::staticCallback(char* topic, byte* payload, unsigned int length) {
    if (_instance) {
        _instance->handleCallback(topic, payload, length);
    }
}

void MqttClient::handleCallback(char* topic, byte* payload, unsigned int length) {
    // Debug: Always print the raw topic received
    Serial.print("*** Callback triggered for topic: ");
    Serial.println(topic);
    Serial.print("Payload length: ");
    Serial.println(length);

    // Create a temporary buffer to safely null-terminate the payload
    char messagePayload[length + 1];
    memcpy(messagePayload, payload, length);
    messagePayload[length] = '\0'; // Safely null-terminate

    // Debug: Print strcmp results for known topics
    Serial.print("strcmp(topic, MQTT_TOPIC_SCHEDULE_SET): ");
    Serial.println(strcmp(topic, MQTT_TOPIC_SCHEDULE_SET));
    Serial.print("strcmp(topic, MQTT_TOPIC_SCHEDULE_GET): ");
    Serial.println(strcmp(topic, MQTT_TOPIC_SCHEDULE_GET));
    Serial.print("strcmp(topic, MQTT_TOPIC_TEST): ");
    Serial.println(strcmp(topic, MQTT_TOPIC_TEST));

    // Use strcmp for exact topic matching, which is safer and more efficient.
    if (strcmp(topic, MQTT_TOPIC_SCHEDULE_SET) == 0) {
        Serial.println("MATCH: Received new schedule payload.");
        // The payload is the JSON string for the schedule.
        if (_scheduler->updateSchedule(messagePayload)) {
            Serial.println("Schedule updated successfully.");
            // Optionally, publish the new schedule state back to the broker
            String currentSchedule = _scheduler->getScheduleJson();
            publish(MQTT_TOPIC_SCHEDULE_STATE, currentSchedule.c_str(), false);
        } else {
            Serial.println("Failed to update schedule.");
        }
        return; // Message handled
    }

    if (strcmp(topic, MQTT_TOPIC_SCHEDULE_GET) == 0) {
        Serial.println("Received schedule GET request. Publishing current schedule.");
        String scheduleJson = _scheduler->getScheduleJson();
        // Publish with retain=true to ensure the latest schedule is available
        publish(MQTT_TOPIC_SCHEDULE_STATE, scheduleJson.c_str(), true);
        return; // Message handled
    }

    if (strcmp(topic, MQTT_TOPIC_TEST) == 0) {
        Serial.print("Received message on test topic: ");
        Serial.println(messagePayload);
        return; // Message handled
    }

    // Handle valve commands using sscanf for pattern matching.
    int valveNumber = 0;
    if (sscanf(topic, "smart-garden/valve/%d/command", &valveNumber) == 1) {
        String command = String(messagePayload);
        command.toUpperCase();

        if (valveNumber >= 1 && valveNumber <= 3) {
            char stateTopic[50];
            sprintf(stateTopic, MQTT_TOPIC_VALVE_STATE, valveNumber);

            if (command == "OPEN") {
                _valveController->openValve(valveNumber);
                publish(stateTopic, "open", false);
            } else if (command == "CLOSE") {
                _valveController->closeValve(valveNumber);
                publish(stateTopic, "closed", false);
            } else {
                Serial.printf("Unknown command '%s' for valve %d.\n", command.c_str(), valveNumber);
            }
        } else {
            Serial.printf("Invalid valve number (%d) parsed from topic. Must be 1-3.\n", valveNumber);
        }
        return; // Message handled
    }

    // If the topic was not handled by any of the above, print a warning.
    Serial.print("Warning: Unhandled MQTT topic: ");
    Serial.println(topic);
}