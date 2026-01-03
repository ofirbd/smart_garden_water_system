/**
 * @file Config.h
 * @brief Configuration file for the Smart Garden project.
 * @author ofirbd
 * @version 1.0
 * @date 2025-07-04
 */
#ifndef CONFIG_H
#define CONFIG_H

// =================================================================
//      BOARD CONFIGURATION
// =================================================================
// Comment out the following line to compile for the production board
//#define DEV_BOARD

#ifdef DEV_BOARD
    // --- DEVELOPMENT BOARD CONFIGURATION ---
    #define MQTT_CLIENT_ID "esp32-garden-controller-dev"

    // Use different topics for development to avoid conflicts
    #define MQTT_TOPIC_CONTROLLER_STATUS "smart-garden-dev/controller/status"
    #define MQTT_TOPIC_VALVE_COMMAND "smart-garden-dev/valve/+/command"
    #define MQTT_TOPIC_VALVE_STATE "smart-garden-dev/valve/%d/state"
    #define MQTT_TOPIC_SCHEDULE_SET "smart-garden-dev/schedule/set"
    #define MQTT_TOPIC_SCHEDULE_GET "smart-garden-dev/schedule/get"
    #define MQTT_TOPIC_SCHEDULE_STATE "smart-garden-dev/schedule/state"
    #define MQTT_TOPIC_TEST "test_topic_dev"

#else
    // --- PRODUCTION BOARD CONFIGURATION ---
    #define MQTT_CLIENT_ID "esp32-garden-controller"

    // MQTT Topics
    #define MQTT_TOPIC_CONTROLLER_STATUS "smart-garden/controller/status"
    #define MQTT_TOPIC_VALVE_COMMAND "smart-garden/valve/+/command"
    #define MQTT_TOPIC_VALVE_STATE "smart-garden/valve/%d/state"
    #define MQTT_TOPIC_SCHEDULE_SET "smart-garden/schedule/set"
    #define MQTT_TOPIC_SCHEDULE_GET "smart-garden/schedule/get"
    #define MQTT_TOPIC_SCHEDULE_STATE "smart-garden/schedule/state"
    #define MQTT_TOPIC_TEST "test_topic"

#endif

// =================================================================
//      COMMON CONFIGURATION
// =================================================================
// WiFi Configuration
// IMPORTANT: Replace with your actual WiFi credentials
#define WIFI_SSID "rida1"
#define WIFI_PASSWORD "ofirbendavid1"

// MQTT Configuration
#define MQTT_BROKER_IP "192.168.1.45"
#define MQTT_PORT 1883

// Pin Definitions
#define LED_BUILTIN 2
#define Enable_12V 27

// Valve GPIO Assignments
#define VALVE1_PIN_A 14
#define VALVE1_PIN_B 12
#define VALVE2_PIN_A 16
#define VALVE2_PIN_B 17
#define VALVE3_PIN_A 25
#define VALVE3_PIN_B 26

// Latching valve pulse duration in milliseconds
#define VALVE_PULSE_MS 150

#endif // CONFIG_H
