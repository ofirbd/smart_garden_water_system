#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFiClient.h>
#include "Config.h"
#include "Scheduler.h"
#include "ValveController.h"

class MqttClient {
public:
    MqttClient(Scheduler* scheduler, ValveController* valveController);
    void begin();
    void loop();
    bool publish(const char* topic, const char* payload, bool retained = false);
    bool publish(const char* topic, const uint8_t* payload, unsigned int plength, bool retained = false);
    bool subscribe(const char* topic, uint8_t qos = 0);
    bool connected();

private:
    WiFiClient _espClient;
    PubSubClient _client;
    Scheduler* _scheduler;
    ValveController* _valveController;

    void reconnect();
    static void staticCallback(char* topic, byte* payload, unsigned int length);
    void handleCallback(char* topic, byte* payload, unsigned int length);

    // Static pointer to the MqttClient instance for the static callback
    static MqttClient* _instance;
};

#endif // MQTT_CLIENT_H
