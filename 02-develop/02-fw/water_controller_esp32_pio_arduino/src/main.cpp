/**
 * @file main.cpp
 * @brief Main application file for the Smart Garden Watering System.
 * 
 * This file contains the primary setup and loop functions for the ESP32 controller.
 * It follows best practices by modularizing the application into distinct classes
 * (e.g., MqttClient, Scheduler, ValveController), which are instantiated as global
 * objects. The setup() function initializes these objects and establishes WiFi and
 * MQTT connections. The loop() function is non-blocking and delegates tasks to the
 * respective module's loop() method, ensuring responsive and concurrent operation.
 * This modular approach enhances maintainability, scalability, and readability.
 * 
 * @author ofirbd
 * @version 1.0
 * @date 2025-07-04
 */
#include <Arduino.h>
#include <WiFi.h>
#include "Config.h"
#include "ValveController.h"
#include "TimeManager.h"
#include "Scheduler.h"
#include "LedControl.h"
#include "MqttClient.h"

// Global objects
WiFiClient espClient; // Still needed for WiFi connection
ValveController valveController;
TimeManager timeManager;
Scheduler scheduler(&timeManager, &valveController);
LedControl ledControl(LED_BUILTIN);
MqttClient mqttClient(&scheduler, &valveController);

void setup_wifi() {
  Serial.println("Connecting to WiFi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    // Blink the LED manually while connecting
    ledControl.on();
    delay(250);
    Serial.print(".");
    ledControl.off();
    delay(250);
  }
  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200);
  Serial.println("Program started");
  
  ledControl.begin(); // Initialize LED control

  valveController.begin();
  setup_wifi(); // This will block and blink until mqtt is connected
  timeManager.begin();
  
  // Initialize the scheduler
  scheduler.begin();
  scheduler.setMqttClient(&mqttClient); 

  mqttClient.begin(); // Initialize MQTT client

  // Debug: Print the configured schedule topic to verify its content
  Serial.print("MQTT_TOPIC_SCHEDULE_SET defined as: [");
  Serial.print(MQTT_TOPIC_SCHEDULE_SET);
  Serial.println("]");
}

// For printing time periodically
unsigned long lastTimePrint = 0;

void loop() {
  // The MqttClient loop handles reconnection attempts automatically
  mqttClient.loop();

  // Update LED status based on MQTT connection
  if (mqttClient.connected()) {
    ledControl.on(); // Solid ON when fully connected
  } else {
    ledControl.blink(1000); // Slow blink if MQTT is disconnected
  }

  timeManager.loop();
  
  // Run the scheduler loop
  scheduler.loop(); 

  // Every 5 seconds, print the current time.
  if (millis() - lastTimePrint > 5000) {
    Serial.print("Current time: ");
    Serial.println(timeManager.getFormattedTime());
    lastTimePrint = millis();
  }
}
