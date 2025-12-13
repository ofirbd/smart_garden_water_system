# ESP32 Firmware Detailed Design

## 1. Introduction

This document outlines the detailed design for the firmware of the ESP32-based Smart Garden Irrigation Controller. It is based on the overall System Detailed Design and incorporates the specific hardware interface details provided.

## 2. Development Environment

-   **IDE:** Visual Studio Code with PlatformIO
-   **Framework:** Arduino
-   **Target Boards:**
    -   **Production:** ESP32-WROOM-32D/E (PlatformIO Board ID: `esp32dev`)
    -   **Development:** FireBeetle ESP32 (PlatformIO Board ID: `firebeetle32`)

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
| **Valve 1 Control A** | 14  | OUTPUT    | H-Bridge Input 1 for Valve 1               |
| **Valve 1 Control B** | 12  | OUTPUT    | H-Bridge Input 2 for Valve 1               |
| **Valve 2 Control A** | 16   | OUTPUT    | H-Bridge Input 1 for Valve 2               |
| **Valve 2 Control B** | 17   | OUTPUT    | H-Bridge Input 2 for Valve 2               |
| **Valve 3 Control A** | 25  | OUTPUT    | H-Bridge Input 1 for Valve 3               |
| **Valve 3 Control B** | 26  | OUTPUT    | H-Bridge Input 2 for Valve 3               |
| **Water Flow Sensor** | 13   | INPUT     | (Future) Pulse counting input              |
| **Temp Sensor (1-Wire)**| 15   | INPUT     | (Future) Dallas Temperature Sensor         |
| **Built-in LED**      | 2    | OUTPUT    | (Usually GPIO 2 on most dev boards)        |
| **Enable 12V** | 27 | OUTPUT | Enable 12V for H-Bridge drivers |

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

-   **Blinking (Connecting):** The LED blinks while the device is attempting to connect to WiFi.
-   **Solid ON (Connected):** The device is successfully connected to WiFi and the MQTT Broker, and is operating normally.
-   **Slow Blinking (Error):** The connection to the MQTT broker has been lost.

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
    -   **Important:** The client buffer size must be increased to at least 512 bytes to handle the JSON schedule payload (e.g., `client.setBufferSize(512);`).
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

## 8. Build Configurations and Future Development

This section describes the process for compiling the firmware for different environments and outlines the plan for future multi-controller support.

### 8.1. Development vs. Production Builds

To allow for parallel development and testing without interfering with the main, operational system, the firmware can be compiled in two modes: **Production** and **Development**. This is controlled by a macro in `Config.h`.

The key differences are the `MQTT_CLIENT_ID` and the MQTT topic prefixes, which are isolated from each other.

-   **Production Mode:** Uses the client ID `esp32-garden-controller` and publishes to `smart-garden/...` topics. This is the version that interacts with the live backend and frontend applications.
-   **Development Mode:** Uses the client ID `esp32-garden-controller-dev` and publishes to `smart-garden-dev/...` topics. This allows it to connect to the MQTT broker without disconnecting the main board.

#### How to Switch Modes

1.  Open the file `02-develop/02-fw/water_controller_esp32_pio_arduino/include/Config.h`.
2.  To compile for the **Development Board**, ensure the following line is active:
    ```c++
    #define DEV_BOARD
    ```
3.  To compile for the **Production Board**, comment out that line:
    ```c++
    // #define DEV_BOARD
    ```
4.  Open the file `02-develop/02-fw/water_controller_esp32_pio_arduino/platformio.ini`.
5.  Update the `default_envs` setting to match your target board:
    -   For the **Development Board**: `default_envs = firebeetle32`
    -   For the **Production Board**: `default_envs = esp32dev`

### 8.2. Monitoring the Development Board

Since the Development build uses separate MQTT topics, it will not appear in the official web or mobile applications. You can monitor its activity using a generic MQTT client or the provided test script.

To use the test script:
1.  Open the file `03-testing/test_mqtt/mqtt_garden_monitor.py`.
2.  Set the flag `MONITOR_DEV_BOARD = True`.
3.  Run the script. It will subscribe to the `smart-garden-dev/#` topics and print all messages from your development board.

### 8.3. Future Plan: True Multi-Controller Support

The current `DEV_BOARD` flag is a temporary solution for parallel development. A future goal is to upgrade the entire system to natively support multiple smart water units operating simultaneously as part of a single, unified system.

This will require significant architectural changes:

1.  **Firmware:** The firmware will need to be updated to use a truly unique identifier (e.g., derived from the MAC address) for its MQTT client ID and topic structure, removing the need for compile-time flags.
2.  **Backend:** The backend server will need to be refactored to handle data from an arbitrary number of devices. This includes subscribing to wildcard topics (e.g., `smart-garden/unit/+/status`) and storing and processing all data on a per-device basis.
3.  **Frontend (Web & Mobile):** The user interfaces will need to be redesigned to support multiple devices. This would likely involve adding a device management screen where users can add, remove, and select which device they want to view and control.
