# ESP32 Firmware Detailed Design

## 1. Introduction

This document outlines the detailed design for the firmware of the ESP32-based Smart Garden Irrigation Controller. It is based on the overall System Detailed Design and incorporates the specific hardware interface details provided.

## 2. Development Environment

-   **IDE:** Visual Studio Code with PlatformIO
-   **Framework:** Arduino
-   **Target Boards:** ESP32-FireBeetle or DOIT ESP32 DEVKIT V1

## 3. Core Libraries

-   `<WiFi.h>`: For WiFi connectivity.
-   `<PubSubClient.h>`: For MQTT communication.
-   `<ArduinoJson.h>`: For efficient parsing and generation of JSON data for MQTT payloads.
-   `<NTPClient.h>` or similar: For time synchronization.
-   `<SPIFFS.h>` or `<EEPROM.h>`: For persistent storage of the schedule.

## 4. Hardware Abstraction Layer (HAL)

This section defines the mapping of physical hardware to the firmware logic.

### 4.1. GPIO Pin Assignments

| Function              | GPIO | Direction | Notes                                      |
| --------------------- | ---- | --------- | ------------------------------------------ |
| **Valve 1 Control A** | 25   | OUTPUT    | H-Bridge Input 1 for Valve 1               |
| **Valve 1 Control B** | 26   | OUTPUT    | H-Bridge Input 2 for Valve 1               |
| **Valve 2 Control A** | 16   | OUTPUT    | H-Bridge Input 1 for Valve 2               |
| **Valve 2 Control B** | 17   | OUTPUT    | H-Bridge Input 2 for Valve 2               |
| **Valve 3 Control A** | 12   | OUTPUT    | H-Bridge Input 1 for Valve 3               |
| **Valve 3 Control B** | 14   | OUTPUT    | H-Bridge Input 2 for Valve 3               |
| **Water Flow Sensor** | 13   | INPUT     | (Future) Pulse counting input              |
| **Temp Sensor (1-Wire)**| 15   | INPUT     | (Future) Dallas Temperature Sensor         |
| **Built-in LED**      | 2    | OUTPUT    | (Usually GPIO 2 on most dev boards)        |

### 4.2. Latching Valve Control Logic

To control the latching valves via an H-Bridge, a short pulse is required. The polarity of the pulse determines the action:

-   **To Open:**
    1.  Set Control Pin A HIGH.
    2.  Set Control Pin B LOW.
    3.  Wait for a short duration (e.g., 100-200ms).
    4.  Set both pins LOW to conserve power.
-   **To Close:**
    1.  Set Control Pin A LOW.
    2.  Set Control Pin B HIGH.
    3.  Wait for a short duration (e.g., 100-200ms).
    4.  Set both pins LOW.

This logic will be encapsulated within the `ValveController` module.

### 4.3. Status LED Logic

-   **Solid ON:** The firmware has started, and the `setup()` function has completed. The device is attempting to connect to WiFi.
-   **Blinking (1Hz):** The device is successfully connected to WiFi and the MQTT Broker and is operating normally.
-   **Fast Blinking (5Hz):** An error state or a specific process is running (e.g., captive portal for WiFi config).

## 5. Firmware Architecture

The firmware will be composed of several distinct modules (classes) to ensure modularity and ease of maintenance.

-   **`Config.h` (Header File)**
    -   A centralized header for all static configuration.
    -   **Contents:**
        -   WiFi SSID and Password (can be placeholders for a WiFi manager).
        -   MQTT Broker IP Address: `192.168.1.45`
        -   MQTT Port: `1883`
        -   MQTT Topic definitions.
        -   GPIO pin definitions as `const int`.

-   **`main.cpp`**
    -   **`setup()`:** Initializes Serial, GPIOs, sets LED to solid ON, initializes and starts WiFiManager, MqttClient, TimeManager, and Scheduler.
    -   **`loop()`:** The main non-blocking loop. It will call the `loop()` methods of all active modules (e.g., `MqttClient.loop()`, `Scheduler.check()`).

-   **`WiFiManager` Module**
    -   Responsible for connecting to the local WiFi network.
    -   Handles automatic reconnection if the connection is lost.
    -   (Future enhancement: Implement a captive portal for first-time setup).

-   **`MqttController` Module**
    -   Manages the connection to the MQTT broker at `192.168.1.45`.
    -   Uses the `PubSubClient` library.
    -   **On connection:**
        -   Publishes `online` to `smart-garden/controller/status` (using LWT).
        -   Subscribes to command topics: `smart-garden/valve/+/command` and `smart-garden/schedule/set`.
    -   **Callback Function:**
        -   Parses incoming messages. Uses `ArduinoJson` to decode schedule payloads.
        -   Calls `ValveController` functions based on valve commands.
        -   Calls `Scheduler` functions to update the schedule.
    -   Provides public methods to publish status (e.g., `publishValveState(int valve, const char* state)`).

-   **`ValveController` Module**
    -   Abstracts the physical valve control.
    -   **Public Methods:** `openValve(int valveNum)`, `closeValve(int valveNum)`, `getValveState(int valveNum)`.
    -   **Internal Logic:** Implements the H-Bridge pulse logic as described in section 4.2. Manages the state (`open`, `closed`) of each valve.

-   **`Scheduler` Module**
    -   Manages the watering schedule.
    -   **Storage:** Loads the schedule from SPIFFS/NVS into memory on startup. Saves any new schedule received via MQTT to persistent storage.
    -   **`check()` method:** Called repeatedly from the main `loop()`. Compares the current time (from TimeManager) with the stored schedule. If a watering event is due, it calls the appropriate `ValveController` method (`openValve`). It also schedules a corresponding `closeValve` action after the specified duration.
    -   Uses the JSON structure defined in the system design for schedule data.

## 6. MQTT Communication Details

-   **Broker:** `192.168.1.45`, Port `1883`
-   **Client ID:** `esp32-garden-controller`
-   **Last Will & Testament (LWT):**
    -   **Topic:** `smart-garden/controller/status`
    -   **Message:** `offline`
    -   **QoS:** 1, Retained: `true`
-   **Primary Subscriptions:**
    -   `smart-garden/valve/+/command`
    -   `smart-garden/schedule/set`
    -   `smart-garden/schedule/get`
-   **Primary Publications:**
    -   `smart-garden/controller/status`
    -   `smart-garden/valve/+/state`
    -   `smart-garden/schedule/state`

## 7. Data Persistence

-   The watering schedule received via MQTT will be stored in the ESP32's non-volatile storage (NVS) or in a JSON file on the SPIFFS filesystem.
-   This ensures that the schedule is not lost if the device reboots or loses power.
-   The `Scheduler` module will be responsible for all read/write operations to persistent storage.
