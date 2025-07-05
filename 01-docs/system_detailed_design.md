# System Detailed Design: Smart Garden Watering System

## 1. Introduction

This document provides a detailed design for the Smart Garden Watering System. It covers the hardware, firmware, and software components, outlining their architecture, interactions, and implementation details. The system is designed to be modular and extensible for future additions like sensors.

## 2. System Architecture (High-Level)

The architecture remains similar, with the combined ESP32 unit now including temperature and humidity sensors.

```
+-------------------+             +-----------------------+
|                   |             |                       |
| Mobile Application| <---------> | Central Control Unit  |
| (Android/iOS)     | (MQTT/HTTP) | (Raspberry Pi)        |
|                   |             | - MQTT Broker         |
+-------------------+             | - Data Storage        |
                                  | - API/Web Server      |
                                  +-----------+-----------+
                                              |
                                              | MQTT
                                              |
                                  +-----------+-----------+
                                  |                       |
                                  | Combined Smart Water  |
                                  | Unit (ESP32)          |
                                  | - 3x Valve Actuation  |
                                  | - 1x Water Flow Sensor|
                                  | - Temp/Hum Sensors    |
                                  | - Status Reporting    |
                                  | - Data Reporting      |
                                  | - Battery/Power       |
                                  +-----------------------+
```

The system consists of the following main components:

1.  **Combined Smart Water Unit (ESP32):** This is the edge device responsible for direct interaction with the garden. It includes:
    *   **Valve Actuation:** Controls three water valves.
    *   **Water Flow Sensor:** Monitors water consumption.
    *   **Temperature/Humidity Sensors:** Gathers environmental data for smarter watering decisions.
    *   **Status & Data Reporting:** Communicates its operational status, sensor readings, and battery/power information.

2.  **Central Control Unit (Raspberry Pi):** This acts as the central hub of the system, typically hosted on a Raspberry Pi. Its functions include:
    *   **MQTT Broker:** Facilitates real-time, asynchronous communication between all components.
    *   **Data Storage:** Stores historical data from sensors and system events.
    *   **API/Web Server:** Provides an interface for the mobile application to interact with the system.

3.  **Mobile Application:** A cross-platform application (Android/iOS, or a mobile-friendly web app) that serves as the user interface for monitoring and control. It allows users to:
    *   View real-time system status (valve states, sensor data).
    *   Manually control individual valves.
    *   Manage and update watering schedules.

**Communication Flow:**

*   The **Combined Smart Water Unit (ESP32)** connects to the home WiFi and communicates with the **Central Control Unit (Raspberry Pi)** via MQTT. It subscribes to command topics (e.g., for valve control, schedule updates) and publishes its status, sensor data, and water usage information.
*   The **Mobile Application** communicates with the **Central Control Unit (Raspberry Pi)**. This communication can be via MQTT directly or, more commonly, through an API/Web Server on the Raspberry Pi that then interacts with the MQTT Broker. This setup prevents exposing MQTT credentials directly to the mobile app and allows for more complex data processing and presentation.
*   The user primarily interacts with the **Mobile Application** to manage and monitor the entire system.

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
-   **Frontend:** A simple, responsive single-page application (SPA) built with **React**. It provides a dynamic user experience for interacting with the system.
-   **Backend/BFF (Backend For Frontend):** A lightweight backend implemented with **Node.js and Express.js**. This backend serves the React frontend, manages a persistent connection to the MQTT broker, and acts as a WebSocket server to relay real-time MQTT messages to the frontend. This architecture prevents exposing MQTT credentials directly to the browser.

### 5.2. UI Components
-   **Dashboard:** The primary view displaying the real-time status of the system. It shows:
    -   Controller status (online/offline).
    -   Current state of each valve (open/closed).
    -   Manual control buttons to open or close individual valves.
-   **Schedule Editor:** An intuitive interface for managing watering schedules. Users can:
    -   View the current schedule loaded from the ESP32.
    -   Add new watering programs.
    -   Edit existing programs, including enabling/disabling, selecting days of the week, setting start times, and defining watering durations for specific valves.
    -   Save updated schedules, which are then sent to the ESP32 for persistence.

## 6. Mobile Web Application

A mobile-friendly web application, built with Flutter, provides a streamlined experience for users on mobile devices. It is a Progressive Web App (PWA) that can be accessed through any modern mobile browser, offering a native-like feel without the need for app store installation.

### 6.1. Technology Stack
-   **Framework:** Flutter Web.
-   **State Management:** Provider or Riverpod for managing application state.
-   **Communication:** Direct WebSocket connection to the backend server to receive real-time updates and send commands.

### 6.2. Key Features
-   **Dashboard:** A mobile-optimized view showing controller status and valve states.
-   **Schedule Editor:** A touch-friendly interface for managing watering schedules.
-   **Responsive Design:** The UI is designed to work seamlessly on various screen sizes.

### 6.3. Operation

The mobile web application is served directly by the Node.js backend. To access the application:

1.  Ensure the Node.js backend server is running.
2.  Open a web browser on your mobile device or computer.
3.  Navigate to the address where your backend server is hosted (e.g., `http://<your_raspberry_pi_ip>:3001`).

The application will load in your browser, providing a mobile-friendly interface for monitoring and controlling your smart garden system.

## 7. Web Application Operation

The original web application consists of a Node.js backend and a React frontend, which need to be run separately.

### 7.1. Directory Structure
The web application code is located under `02-develop/03-sw/web-app/`:
-   `02-develop/03-sw/web-app/backend/`: Contains the Node.js Express server.
-   `02-develop/03-sw/web-app/frontend/`: Contains the React single-page application.

### 7.2. Starting the Backend Server
1.  Open a terminal or command prompt.
2.  Navigate to the backend directory:
    ```bash
    cd 02-develop/03-sw/web-app/backend
    ```
3.  Install dependencies (if not already done, or if `package.json` has changed):
    ```bash
    npm install
    ```
4.  Start the server:
    ```bash
    npm start
    ```
    The backend server will typically run on `http://localhost:3001`. Keep this terminal open.

### 7.3. Starting the Frontend Application
1.  Open a *separate* terminal or command prompt.
2.  Navigate to the frontend directory:
    ```bash
    cd 02-develop/03-sw/web-app/frontend
    ```
3.  Install dependencies (if not already done, or if `package.json` has changed):
    ```bash
    npm install
    ```
4.  Start the development server:
    ```bash
    npm start
    ```
    This will usually open a new browser tab at `http://localhost:3000` (or another available port) where your React application will be running.

### 7.4. Communication Flow
-   The React frontend communicates with the Node.js backend via HTTP API calls (for sending commands) and WebSockets (for receiving real-time MQTT updates).
-   The Node.js backend maintains a persistent connection to the MQTT broker.
-   When the frontend sends a command (e.g., open valve, save schedule), it makes an HTTP POST request to the backend's `/api/command` endpoint. The backend then publishes this command to the appropriate MQTT topic.
-   The backend subscribes to relevant MQTT topics (e.g., controller status, valve states, schedule state). Any messages received from the MQTT broker are immediately relayed to all connected frontend clients via WebSocket.
-   Upon a new WebSocket connection from the frontend, the backend immediately sends the last known retained states for controller status and schedule to ensure the frontend displays up-to-date information without delay.

### 7.5. Key Files
-   `02-develop/03-sw/web-app/backend/server.js`: Main backend server logic.
-   `02-develop/03-sw/web-app/backend/package.json`: Backend dependencies.
-   `02-develop/03-sw/web-app/frontend/src/App.js`: Main React application component, handles routing and overall state.
-   `02-develop/03-sw/web-app/frontend/src/ScheduleEditor.js`: React component for schedule management.
-   `02-develop/03-sw/web-app/frontend/package.json`: Frontend dependencies and proxy configuration.
