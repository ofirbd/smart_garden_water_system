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
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200);
  ledControl.begin(); // Initialize LED control
  ledControl.on(); // Turn LED ON during setup

  valveController.begin();
  setup_wifi();
  timeManager.begin();
  
  // Initialize the scheduler
  scheduler.begin(); 

  mqttClient.begin(); // Initialize MQTT client

  // Debug: Print the configured schedule topic to verify its content
  Serial.print("MQTT_TOPIC_SCHEDULE_SET defined as: [");
  Serial.print(MQTT_TOPIC_SCHEDULE_SET);
  Serial.println("]");

  ledControl.off(); // Turn LED OFF before loop
}

// For printing time periodically
unsigned long lastTimePrint = 0;

void loop() {
  if (!mqttClient.connected()) {
    // MqttClient handles its own reconnection
  }
  mqttClient.loop();
  timeManager.loop();
  
  // Run the scheduler loop
  scheduler.loop(); 

  // Every 5 seconds, print the current time.
  if (millis() - lastTimePrint > 5000) {
    Serial.print("Current time: ");
    Serial.println(timeManager.getFormattedTime());
    lastTimePrint = millis();
  }

  // Blink LED to show normal operation
  ledControl.blink(500); // Blink every 500ms
}
