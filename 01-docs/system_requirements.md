## Smart Garden Water System

## Description & Requirements

This document outlines the refined requirements for your smart garden water system, detailing a single ESP32 unit that controls three water valves, reads a single water meter, and now also incorporates temperature and humidity sensors. The system still leverages a Raspberry Pi as the central control unit and a mobile application for enhanced control and monitoring.

### 1. System Overview

The system aims to automate garden watering and provide real-time monitoring and control, now enhanced with environmental data.

**Key Features (Enhanced):**

- **Automated Watering:** Schedule-based and sensor-driven watering, now incorporating temperature and humidity data for smarter decisions.
- **Manual Control:** On-demand control of individual valves via the mobile app.
- **Water Usage Monitoring:** Real-time and historical water consumption data.
- **System Status:** Real-time feedback on valve states, connectivity, and potential issues.
- **Environmental Monitoring:** Real-time temperature and humidity readings.
- **Notifications:** Alerts for critical events (e.g., low battery, leak detection - if implemented) and potentially for extreme temperature/humidity conditions.
- **Scalability:** Ability to add more valves and meters (potentially necessitating another combined ESP32 unit).
- **User-Friendly Interface:** Intuitive mobile application for control, monitoring, and configuration.

### 2. System Architecture (High-Level - Refined)

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

### 3. Detailed Requirements

#### 3.1. Combined Smart Water Unit (Single ESP32)

This ESP32 will manage 3 valves, 1 water meter, and temperature/humidity sensors.

**Hardware Requirements (Updated):**

- Microcontroller:

   ESP32 (e.g., ESP32-WROOM-32 or ESP32-S3). Ensure enough GPIO pins for 3 valves + 1 flow sensor input + temperature/humidity sensors + power/status LEDs.

  - **GPIO Allocation:** Even more critical now. You'll need at least 3 output pins for valves, 1 input pin for the flow sensor, and potentially 2-3 pins for the temperature/humidity sensor (depending on the sensor type).

- **Valve Actuators (3x):** Latching solenoid valves (preferred) or motor-driven valves.

- **Water Flow Sensor (1x):** Hall-effect flow sensor or turbine flow meter.

- Temperature/Humidity Sensor:

  - **DHT22 or AM2302:** Relatively inexpensive, but less accurate and slower. Requires one GPIO pin.
  - **BME280:** More accurate and faster, also measures pressure. Uses I2C (2 pins: SDA, SCL).
  - **SHT31:** High accuracy, uses I2C.

- **Power Supply:** Battery-powered (recommended) or mains powered.

- **Enclosure:** IP65/IP67 rated waterproof enclosure.

- **Connectors:** Waterproof connectors.

- **Status LEDs:**

- **Optional:** Manual override buttons.

**Software Requirements (C++ on ESP-IDF or Arduino IDE) (Updated):**

- **Wi-Fi Connectivity:**
- MQTT Client:
  - Subscribe Topics:
    - `garden/unit/<unit_id>/valve/<valve_id>/command`
    - `garden/unit/<unit_id>/schedule`
  - Publish Topics:
    - `garden/unit/<unit_id>/valve/<valve_id>/status`
    - `garden/unit/<unit_id>/meter/flow_rate`
    - `garden/unit/<unit_id>/meter/total_volume`
    - `garden/unit/<unit_id>/meter/session_volume`
    - `garden/unit/<unit_id>/temperature`: Temperature in Celsius.
    - `garden/unit/<unit_id>/humidity`: Relative humidity (percentage).
    - `garden/unit/<unit_id>/battery`
    - `garden/unit/<unit_id>/heartbeat`
- **Valve Control Logic:**
- **Flow Measurement Logic:**
- Temperature/Humidity Measurement Logic:
  - Readings from the chosen sensor.
  - Consider filtering or averaging to reduce noise.
  - Report temperature in Celsius.
  - Report relative humidity in percentage.
- Data Aggregation & Publishing Strategy:
  - **Flow Data:**
  - **Total Volume:**
  - **Valve Status:**
  - **Battery:**
  - **Temperature/Humidity:** Publish these readings periodically (e.g., every 5-15 minutes) as they don't change very rapidly.
- **Power Management:**
- **OTA Updates:**
- **Configuration:**
- **Error Handling:**

#### 3.2. Central Control Unit (Raspberry Pi)

The Raspberry Pi requirements are updated to handle the new temperature and humidity data.

**Software Requirements (Key updates for topics):**

- **MQTT Broker:** Mosquitto MQTT Broker.

- Data Storage:

   PostgreSQL/MySQL/InfluxDB.

  - Schema Design (Updated):
    - `devices` table: `device_id`, `device_name`, `location`, `type`.
    - `valves` table: `valve_id`, `device_id` (FK), `valve_name`, `zone`, `current_state`, `last_updated`.
    - `meters` table: `meter_id`, `device_id` (FK), `meter_name`, `location`, `total_volume`, `last_updated`.
    - `valve_logs` table: `timestamp`, `valve_id`, `action`, `source`.
    - `meter_readings` table: `timestamp`, `meter_id`, `flow_rate`, `session_volume`, `total_volume_at_reading`.
    - `environment_readings` table: `timestamp`, `device_id`, `temperature`, `humidity`.
    - `schedules` table: `schedule_id`, `valve_id`, `start_time`, `duration`, `days_of_week`, `active`.

- Backend Application/API (Python - Flask/FastAPI, Node.js - Express, or Go):

  - MQTT Client:

     Subscribe to:

    - `garden/unit/<unit_id>/#`

  - Data Ingestion:

    - When `garden/unit/<unit_id>/valve/<valve_id>/status` arrives, update `valves.current_state` and log to `valve_logs`.
    - When `garden/unit/<unit_id>/meter/...` arrives, insert into `meter_readings` and update `meters.total_volume`.
    - When `garden/unit/<unit_id>/temperature` and `garden/unit/<unit_id>/humidity` arrive, insert into `environment_readings`.
    - Store `battery` status.

  - API Endpoints (Updated):

    - `/api/v1/devices`: GET.
    - `/api/v1/devices/<device_id>/valves`: GET.
    - `/api/v1/valves/<valve_id>`: GET, PUT.
    - `/api/v1/devices/<device_id>/meter/current`: GET.
    - `/api/v1/devices/<device_id>/meter/history`: GET.
    - `/api/v1/devices/<device_id>/environment/current`: GET (latest temperature and humidity).
    - `/api/v1/devices/<device_id>/environment/history`: GET (historical temperature and humidity).
    - `/api/v1/schedules`: GET, POST, PUT, DELETE.
    - Authentication/Authorization.

  - **Scheduling Engine:**

  - **Error Handling/Logging:**

  - **System Monitoring:**

#### 3.3. Mobile Application (Android/iOS)

The mobile app will now display temperature and humidity data and potentially use it in scheduling.

**Functionality Requirements (Key updates):**

- Dashboard:
  - Show "Combined Garden Unit" status.
  - Overview of the 3 valves.
  - Summary of water consumption.
  - Display current temperature and humidity.
- Valve Control Screen:
  - List the 3 valves.
  - Toggle buttons.
  - Display battery status.
- Water Meter Data Screen:
  - Display current flow rate.
  - Display total volume.
  - Display session volume.
  - Graphical representation of water consumption.
- Environment Data Screen (New):
  - Display current temperature and humidity.
  - Graphical representation of historical temperature and humidity.
- Scheduling Screen (Enhanced):
  - Create, edit, and delete schedules.
  - Select valves.
  - **Optional:** Allow schedules to be influenced by temperature and humidity (e.g., "Don't water if humidity is above 80%" or "Water for 50% longer if temperature is above 30°C").
- **Configuration/Settings Screen:**

### 4. Development Stages & Considerations

The development stages remain similar.

1. **Hardware Prototyping & Testing (Combined ESP32 Unit):**
   - Even more critical GPIO allocation.
   - Test concurrent operation of valves, flow sensor, and temperature/humidity sensors.
   - Power consumption is *very* important.
   - Enclosure must accommodate all sensors.
2. **ESP32 Firmware Development:**
   - Develop combined MQTT publishing.
   - Implement concurrent control of valves.
   - Ensure data persistence for `total_volume`.
   - Master deep sleep and wake-up.

This enhanced design allows for even smarter and more efficient watering based on environmental conditions. Remember to carefully consider your sensor choices and GPIO allocation!