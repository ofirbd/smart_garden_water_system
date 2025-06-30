# System Detailed Design: Smart Garden Watering System

## 1. Introduction

This document provides a detailed design for the Smart Garden Watering System. It covers the hardware, firmware, and software components, outlining their architecture, interactions, and implementation details. The system is designed to be modular and extensible for future additions like sensors.

## 2. System Architecture

The system consists of three main components:

1.  **ESP32 Irrigation Controller:** The hardware device responsible for controlling the water valves based on a schedule or manual commands.
2.  **MQTT Broker:** A central messaging server (running on a Raspberry Pi) that facilitates communication between the controller and the web application.
3.  **Web Application:** A user interface for monitoring the system, controlling the valves manually, and managing the irrigation schedule.

The communication flow is as follows:
- The **ESP32 Controller** connects to the home WiFi and the MQTT Broker. It subscribes to command topics and publishes its status and sensor data.
- The **Web Application** connects to the MQTT Broker to send commands and receive status updates from the controller.
- The user interacts only with the **Web Application**.

## 3. Hardware Detailed Design

### 3.1. Microcontroller
-   **Board:** ESP32-based board (e.g., FireBeetle ESP32, NodeMCU-32S).
-   **Core Features:** WiFi and Bluetooth capabilities, sufficient GPIOs, low power consumption.

### 3.2. Valve Interface
-   **Valves:** 3 x Latching Solenoid Valves (e.g., 9V).
-   **Control Circuit:** Latching valves require a short pulse of DC power to open and a pulse of the *reverse polarity* to close. A simple relay is insufficient. An **H-Bridge Motor Driver** is required for each valve to easily reverse the polarity of the pulse.
-   **Recommended Driver:** A dual H-Bridge driver like the **DRV8833** can control two valves. Therefore, two DRV8833 modules or a similar motor driver IC that can handle 3 channels will be used.
-   **Wiring:** The ESP32 will send two signals per valve to the H-Bridge driver: one for direction (polarity) and one for the pulse (enable).

### 3.3. Power Supply
-   The system will be powered by a wall adapter (e.g., 9V or 12V DC).
-   A buck converter will be used to step down the voltage to 5V/3.3V to power the ESP32 and other logic components. The main voltage will be used to drive the valves via the H-bridge.

### 3.4. Future Sensor Integration
-   **Temperature Sensor:** A DS18B20 waterproof temperature sensor can be connected to a GPIO pin.
-   **Water Flow Sensor:** A hall-effect water flow sensor (e.g., YF-S201) can be connected to a pulse-counting capable GPIO pin.

## 4. Firmware Detailed Design (ESP32)

### 4.1. Development Environment
-   **IDE:** Visual Studio Code with the PlatformIO extension.
-   **Framework:** Arduino.

### 4.2. Firmware Modules
The firmware will be modular to ensure separation of concerns.

-   **`main.cpp`**: Main entry point. Initializes and coordinates all other modules.
-   **`WiFiManager.h/cpp`**: Handles WiFi connection, reconnection logic, and credential management (potentially using a captive portal for initial setup).
-   **`MqttClient.h/cpp`**: Manages connection to the MQTT broker, handles subscriptions, publishing, and message parsing. It will be responsible for all communication logic.
-   **`ValveController.h/cpp`**: Provides a high-level API to control the valves (e.g., `openValve(int valveNumber)`, `closeValve(int valveNumber)`). This module will abstract the low-level H-Bridge control logic.
-   **`Scheduler.h/cpp`**: Manages the watering schedule. It will store schedule data in non-volatile memory (SPIFFS or EEPROM), check the current time, and trigger watering events by calling the `ValveController`.
-   **`TimeManager.h/cpp`**: Connects to an NTP server to fetch and synchronize the real-time clock (RTC).

### 4.3. MQTT Topic Structure
A clear and hierarchical topic structure will be used:

-   **Controller Status:**
    -   `smart-garden/controller/status` -> `online` or `offline` (published by controller using Last Will and Testament).
-   **Valve Control (Commands):**
    -   `smart-garden/valve/1/command` -> `OPEN` or `CLOSE` (published by web app).
    -   `smart-garden/valve/2/command` -> `OPEN` or `CLOSE`.
    -   `smart-garden/valve/3/command` -> `OPEN` or `CLOSE`.
-   **Valve State (Feedback):**
    -   `smart-garden/valve/1/state` -> `open`, `closed`, `opening`, `closing` (published by controller).
    -   `smart-garden/valve/2/state` -> `open`, `closed`, `opening`, `closing`.
    -   `smart-garden/valve/3/state` -> `open`, `closed`, `opening`, `closing`.
-   **Schedule Management:**
    -   `smart-garden/schedule/set` -> JSON payload with the schedule data (published by web app).
    -   `smart-garden/schedule/get` -> Empty message to request the current schedule (published by web app).
    -   `smart-garden/schedule/state` -> JSON payload with the current schedule (published by controller in response to `get`).
-   **Sensor Data (Future):**
    -   `smart-garden/sensor/temperature/celsius` -> `25.5`.
    -   `smart-garden/sensor/water-flow/lpm` -> `10.2`.

### 4.4. Data Structures
-   **Schedule JSON:** The schedule will be represented as a JSON object.
    ```json
    {
      "enabled": true,
      "programs": [
        {
          "enabled": true,
          "days": ["Monday", "Wednesday", "Friday"],
          "actions": [
            { "valve": 1, "time": "08:00", "duration_minutes": 15 },
            { "valve": 2, "time": "08:20", "duration_minutes": 10 }
          ]
        },
        {
          "enabled": true,
          "days": ["Sunday"],
          "actions": [
            { "valve": 3, "time": "19:00", "duration_minutes": 30 }
          ]
        }
      ]
    }
    ```

## 5. Web Application Detailed Design

### 5.1. Technology Stack
-   **Frontend:** A simple, responsive single-page application (SPA) using **React or Vue.js** for a dynamic user experience.
-   **Backend/BFF:** A lightweight backend (e.g., **Node.js with Express**) will serve the frontend and manage a persistent connection to the MQTT broker via WebSockets to the client. This avoids exposing MQTT credentials to the browser.

### 5.2. UI Components
-   **Dashboard:** Main view showing the status of each valve (open/closed), controller status (online/offline), and current sensor readings.
-   **Manual Control Panel:** A set of buttons to manually open or close each of the three valves.
-   **Schedule Editor:** An intuitive interface to create, edit, and delete watering programs. It will allow users to select days of the week, set start times, and define durations for each valve.
-   **Settings Page:** For configuring MQTT broker details (if not hardcoded) and other system settings.
