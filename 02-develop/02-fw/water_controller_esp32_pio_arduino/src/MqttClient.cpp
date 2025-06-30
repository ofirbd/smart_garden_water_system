#include "MqttClient.h"

MqttClient* MqttClient::_instance = nullptr; // Initialize static instance pointer

MqttClient::MqttClient(Scheduler* scheduler, ValveController* valveController)
: _client(_espClient), _scheduler(scheduler), _valveController(valveController) {
    _instance = this; // Set the static instance pointer
}

void MqttClient::begin() {
    _client.setServer(MQTT_BROKER_IP, MQTT_PORT);
    _client.setCallback(MqttClient::staticCallback);
}

void MqttClient::loop() {
    if (!_client.connected()) {
        reconnect();
    }
    _client.loop();
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
    while (!_client.connected()) {
        Serial.print("Attempting MQTT connection...");
        if (_client.connect(MQTT_CLIENT_ID, MQTT_TOPIC_CONTROLLER_STATUS, 1, true, "offline")) {
            Serial.println("connected");
            _client.publish(MQTT_TOPIC_CONTROLLER_STATUS, "online", true);
            subscribe(MQTT_TOPIC_VALVE_COMMAND);
            if (subscribe(MQTT_TOPIC_SCHEDULE_SET, 1)) {
                Serial.println("Subscribed to schedule topic successfully.");
            } else {
                Serial.println("Failed to subscribe to schedule topic.");
            }
            if (subscribe(MQTT_TOPIC_SCHEDULE_GET, 1)) {
                Serial.println("Subscribed to schedule GET topic successfully.");
            } else {
                Serial.println("Failed to subscribe to schedule GET topic.");
            }
            // Request the current schedule from the broker
            publish(MQTT_TOPIC_SCHEDULE_GET, (const uint8_t*)"", 0, false); // QoS 1, not retained, payload length 0
            Serial.println("Requested schedule from broker.");
            // Force client.loop() for a few seconds to receive the schedule response
            unsigned long startMillis = millis();
            while (millis() - startMillis < 5000) { // Loop for 5 seconds
                _client.loop();
                delay(10); // Small delay to yield to other tasks
            }
            Serial.println("Finished forced client.loop() for schedule request.");
        } else {
            Serial.print("failed, rc=");
            Serial.print(_client.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
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

    // Create a temporary buffer to safely null-terminate the payload
    char messagePayload[length + 1];
    memcpy(messagePayload, payload, length);
    messagePayload[length] = '\0'; // Safely null-terminate

    // Create a String object from the topic char* for safer comparison
    String topicStr = String(topic);

    // Debug: Print the raw received topic and its length (using strlen for topic)
    Serial.print("Raw received topic (strlen ");
    Serial.print(strlen(topic));
    Serial.print(", payload length ");
    Serial.print(length);
    Serial.print("): [");
    for (unsigned int i = 0; i < strlen(topic); ++i) {
        Serial.print(topic[i]);
    }
    Serial.println("]");

    // Debug: Print the configured schedule topic and its length
    Serial.print("Configured schedule topic (length ");
    Serial.print(strlen(MQTT_TOPIC_SCHEDULE_SET));
    Serial.print("): [");
    Serial.print(MQTT_TOPIC_SCHEDULE_SET);
    Serial.println("]");

    // Handle schedule updates
    if (topicStr == MQTT_TOPIC_SCHEDULE_SET) {
        Serial.println("MATCH: Received new schedule payload.");
        _scheduler->updateSchedule(messagePayload);
        Serial.println("DEBUG: scheduler.updateSchedule() called.");
    }

    // Handle valve commands
    int valveNumber = 0;
    int parsedItems = sscanf(topic, "smart-garden/valve/%d/command", &valveNumber);

    if (parsedItems == 1) {
        String commandStr = String(messagePayload);
        commandStr.toUpperCase();

        if (valveNumber >= 1 && valveNumber <= 3) {
            char stateTopic[50];
            sprintf(stateTopic, MQTT_TOPIC_VALVE_STATE, valveNumber);

            if (commandStr == "OPEN") {
                _valveController->openValve(valveNumber);
                _client.publish(stateTopic, "open", true);
            } else if (commandStr == "CLOSE") {
                _valveController->closeValve(valveNumber);
                _client.publish(stateTopic, "closed", true);
            } else {
                Serial.printf("Unknown command '%s' for valve %d.\n", commandStr.c_str(), valveNumber);
            }
        } else {
            Serial.printf("Invalid valve number (%d) parsed from topic. Must be 1-3.\n", valveNumber);
        }
    } else if (topicStr == MQTT_TOPIC_SCHEDULE_GET) {
        Serial.println("Received schedule GET request. Publishing current schedule.");
        String scheduleJson = _scheduler->getScheduleJson();
        _client.publish(MQTT_TOPIC_SCHEDULE_SET, (const uint8_t*)scheduleJson.c_str(), scheduleJson.length(), true); // Publish with retain=true
    } else if (topicStr == MQTT_TOPIC_TEST) {
        Serial.print("Received message on test topic: ");
        Serial.println(messagePayload);
    }
}