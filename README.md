# Smart Garden Water System

## Project Overview

This repository contains the software and hardware design for a smart garden system designed to automate watering, monitor water consumption, and track environmental conditions (temperature and humidity). The system leverages an ESP32 microcontroller for edge device control and a Raspberry Pi as a central hub, all controllable via a dedicated mobile application.

Built upon prior experience with ESP32 and MQTT, this project aims to create a robust, scalable, and user-friendly solution for efficient garden management.

## Key Features

* **Automated Watering:** Schedule-based watering for up to 3 individual garden zones.
* **Manual Control:** On-demand opening and closing of each water valve via the mobile app.
* **Water Usage Monitoring:** Real-time and historical tracking of total and session water consumption from a single water meter.
* **Environmental Monitoring:** Live readings and historical data for garden temperature and humidity.
* **System Status:** Real-time feedback on valve states, device connectivity, and battery levels.
* **Notifications:** Alerts for critical events (e.g., low battery, potential issues).
* **User-Friendly Mobile App:** Intuitive interface for control, monitoring, and configuration.

## System Architecture

The system follows a star-topology with a central Raspberry Pi hub communicating with a single, powerful ESP32 edge device.

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

## Components

### 1. Combined Smart Water Unit (ESP32 Firmware & Hardware)

This single ESP32 board manages all direct interactions with the garden.

* **Microcontroller:** ESP32 (e.g., ESP32-WROOM-32 or similar, ensuring sufficient GPIOs).
* **Valve Actuators:** 3 x Latching Solenoid Valves (recommended for their minimal power consumption) with appropriate driver circuits (e.g., MOSFET H-bridge for polarity switching).
* **Water Flow Sensor:** 1 x Hall-effect flow sensor (e.g., YF-S201C or equivalent), connected to an interrupt-capable GPIO.
* **Temperature/Humidity Sensor:** [Specify your chosen sensor, e.g., BME280 (I2C) or DHT22 (1-wire)].
* **Power Supply:** Designed for outdoor deployment, likely battery-powered (e.g., 18650 Li-ion cells) with a robust charging circuit (e.g., solar panel + charge controller) and efficient voltage regulation. Alternatively, mains power if available.
* **Enclosure:** IP65/IP67 rated waterproof enclosure to protect electronics from elements.
* **Firmware:** Developed in C++ using the [ESP-IDF framework / Arduino IDE].

### 2. Central Control Unit (Raspberry Pi Software & Hardware)

The brain of the system, handling data, scheduling, and communication with the mobile app.

* **Hardware:** Raspberry Pi [Specify model, e.g., 3B+, 4, or 5].
* **Operating System:** Raspberry Pi OS (formerly Raspbian).
* **MQTT Broker:** Mosquitto (for reliable message queuing between devices).
* **Database:** SQLite (chosen for its lightweight nature, ease of setup, and suitability for the project's data scale).
    * Stores sensor readings, valve states, schedules, and device information.
* **Backend Application/API:** A custom application written in [Python (e.g., Flask/FastAPI), Node.js (e.g., Express), or Go].
    * Listens to MQTT messages, stores data in SQLite, implements scheduling logic, and exposes a RESTful API for the mobile application.
* **Web Tool (Optional but Recommended):** [Specify your chosen tool, e.g., Home Assistant, Node-RED, or ThingsBoard].
    * Provides a ready-made web interface for dashboards, charts, and rule-based automation, significantly reducing development time for the UI.

### 3. Mobile Application

The primary user interface for controlling and monitoring the garden from anywhere.

* **Platform:** Compatible with Android and/or iOS devices.
* **Technology Stack:** [Specify, e.g., React Native, Flutter, Native Kotlin/Swift].
* **Functionality:**
    * Dashboard view with current status and summarized data.
    * Individual valve control (on/off).
    * Display of current and historical water meter readings.
    * Display of current and historical temperature and humidity data.
    * Creation, editing, and deletion of watering schedules per valve.
    * System configuration settings.

## Detailed System Description and Requirements

For an in-depth understanding of the system's complete architecture, detailed hardware and software requirements for each component, precise communication protocols, and in-depth design considerations, please refer to the dedicated **[SYSTEM_REQUIREMENTS.md](SYSTEM_REQUIREMENTS.md)** document. This comprehensive document also outlines potential future enhancements and specific implementation details.

## Setup & Installation

Follow these steps to get your Smart Garden Water System up and running:

### 1. Raspberry Pi (Central Control Unit) Setup

1.  **Prepare Raspberry Pi OS:**
    * Download and flash the latest Raspberry Pi OS (Lite or Desktop, depending on your preference) onto a microSD card using [Raspberry Pi Imager](https://www.raspberrypi.com/software/).
    * Enable SSH for remote access.
2.  **Install Mosquitto MQTT Broker:**
    ```bash
    sudo apt update
    sudo apt install mosquitto mosquitto-clients
    sudo systemctl enable mosquitto
    sudo systemctl start mosquitto
    # For security, consider configuring MQTT users and passwords (refer to Mosquitto docs).
    ```
3.  **Setup Backend Application / Web Tool:**
    * **If using a custom backend:**
        ```bash
        git clone [Your_Repository_URL]
        cd [Your_Repository_Name]/backend/
        # Install Python dependencies (example for Python)
        pip install -r requirements.txt
        # Run the backend application (e.g., using Gunicorn for production)
        python app.py
        ```
    * **If using Home Assistant:** Follow the official installation guide (e.g., Home Assistant OS, Container, or Supervised for Raspberry Pi). Ensure MQTT integration is configured to connect to your Mosquitto broker.
    * **If using Node-RED:** Install via the recommended script for Raspberry Pi: `bash <(curl -sL https://raw.githubusercontent.com/node-red/linux-installers/master/deb/update-nodejs-and-nodered)` Then install necessary nodes (e.g., `node-red-contrib-mqtt`, `node-red-node-sqlite`).
    * **If using ThingsBoard:** Follow the official ThingsBoard Community Edition installation guide for Raspberry Pi (Docker is often the easiest).

### 2. ESP32 (Smart Water Unit) Firmware

1.  **Set up Development Environment:**
    * Install either [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html) or [Arduino IDE with ESP32 board support](https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html).
2.  **Clone Firmware Repository:**
    ```bash
    git clone [Your_Repository_URL]
    cd [Your_Repository_Name]/esp32_firmware/
    ```
3.  **Configure Network & MQTT:**
    * Open the firmware project in your IDE.
    * Locate and edit the configuration file (e.g., `src/config.h`, `secrets.h`, or similar) to enter your Wi-Fi SSID, password, and the IP address/hostname of your Raspberry Pi's MQTT broker.
4.  **Compile and Upload:**
    * Compile the firmware and upload it to your ESP32 board. Ensure correct board and serial port are selected in your IDE.

### 3. Mobile Application

1.  **Clone Mobile App Repository:**
    ```bash
    git clone [Your_Mobile_App_Repository_URL]
    cd [Your_Mobile_App_Repository_Name]
    ```
2.  **Install Dependencies:**
    * For React Native: `npm install` or `yarn install`
    * For Flutter: `flutter pub get`
    * Follow specific instructions for your chosen framework.
3.  **Configure API Endpoint:**
    * In the app's source code, update the backend API base URL to point to your Raspberry Pi's IP address (e.g., `http://[Raspberry_Pi_IP_Address]:[Backend_Port]`).
4.  **Build and Run:**
    * Run the app on an emulator or a physical device:
        * React Native: `npm run android` / `npm run ios`
        * Flutter: `flutter run`
    * Alternatively, build release APK/IPA files for direct installation.

## Usage

1.  **Power On Devices:** Ensure your Raspberry Pi and the ESP32 Smart Water Unit are powered on and connected to your local network.
2.  **Verify MQTT Communication:** Monitor the Mosquitto broker logs (`sudo tail -f /var/log/mosquitto/mosquitto.log`) to confirm the ESP32 is publishing data and subscribing correctly.
3.  **Access Interface:**
    * **Mobile App:** Launch the "Smart Garden Water System" app on your smartphone.
    * **Web Tool:** Open your web browser and navigate to `http://[Raspberry_Pi_IP_Address]:[Web_Tool_Port]` (e.g., `http://192.168.1.100:8123` for Home Assistant, `http://192.168.1.100:1880` for Node-RED, `http://192.168.1.100:8080` for ThingsBoard).
4.  **Control & Monitor:**
    * Use the interface to manually open or close individual water valves.
    * View real-time and historical graphs for water flow, total volume, temperature, and humidity.
    * Create and manage custom watering schedules for your garden zones.
    * Check the battery status and overall health of your ESP32 unit.

## Contributing

Contributions to the Smart Garden Water System are highly welcome! Whether you're interested in bug fixes, feature enhancements, or documentation improvements, please feel free to:

* **Open an issue:** To report bugs, suggest new features, or ask questions.
* **Submit a pull request:** To contribute code changes.

Please refer to the [CONTRIBUTING.md](CONTRIBUTING.md) (if you create one) for detailed guidelines on how to contribute.

## License

This project is open-source and distributed under the [MIT License](LICENSE). See the `LICENSE` file for more details.

---

**[Your Name / GitHub Username]**
**[Date of creation or last significant update - e.g., June 2025]**

