/**
 * @file Config.h
 * @brief Configuration file for the Smart Garden project.
 * @author ofirbd
 * @version 1.0
 * @date 2025-07-04
 */
#ifndef CONFIG_H
#define CONFIG_H

// WiFi Configuration
// IMPORTANT: Replace with your actual WiFi credentials
#define WIFI_SSID "rida1"
#define WIFI_PASSWORD "ofirbendavid1"

// MQTT Configuration
#define MQTT_BROKER_IP "192.168.1.45"
#define MQTT_PORT 1883
#define MQTT_CLIENT_ID "esp32-garden-controller"

// MQTT Topics
#define MQTT_TOPIC_CONTROLLER_STATUS "smart-garden/controller/status"
#define MQTT_TOPIC_VALVE_COMMAND "smart-garden/valve/+/command"
#define MQTT_TOPIC_VALVE_STATE "smart-garden/valve/%d/state"
#define MQTT_TOPIC_SCHEDULE_SET "smart-garden/schedule/set"
#define MQTT_TOPIC_SCHEDULE_GET "smart-garden/schedule/get"
#define MQTT_TOPIC_SCHEDULE_STATE "smart-garden/schedule/state"
#define MQTT_TOPIC_TEST "test_topic"

// Pin Definitions


// Valve GPIO Assignments
#define VALVE1_PIN_A 25
#define VALVE1_PIN_B 26
#define VALVE2_PIN_A 16
#define VALVE2_PIN_B 17
#define VALVE3_PIN_A 12
#define VALVE3_PIN_B 14

// Latching valve pulse duration in milliseconds
#define VALVE_PULSE_MS 150

#endif // CONFIG_H
