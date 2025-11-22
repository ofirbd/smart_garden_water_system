# Hardware Detailed Design: Smart Garden Watering System (Rev 7)

## 1. Introduction

This document provides a detailed hardware design for the Smart Garden Watering System's edge device, referred to as the "Smart Water Unit". This unit is responsible for controlling three 12V latching solenoid valves, monitoring water flow, and measuring environmental conditions. The design prioritizes reliability with a dual power source strategy and efficiency, utilizing a shared boost converter for valve actuation.

## 2. System Block Diagram

```
+-----------------------------------------------------------------+|
|                           Smart Water Unit                        |
|                                                                 |
|  +--------------------------+     +---------------------------+   |
|  |      Power Management    |     |      ESP32-WROOM-32       |   |
|  | (USB 5V/9V Bat -> Switch)+---->|      Microcontroller      |   |
|  |           |              |     |                           |   |
|  |           v              |     +--+------------------+-----+   |
|  | +----------------------+ |        |                  |         |
|  | | 12V Boost Converter  | |        | (EN)             | (I2C)   |
|  | +----------------------+ |        |                  |         |
|  |           |              |     +--v------------------v-----+   |
|  +-----------|--------------+     |    DRV8871 H-Bridge (x3)  |   |
|              | (12V)              |                           |   |
|  +-----------v---------------+    +---------------------------+   |
|  |   Latching Valves (x3)|<---+                                 |
|  | (12V)                 |                                     |
|  +-----------------------+                                     |
|                                                                 |
+-----------------------------------------------------------------+
```

## 3. Component Selection

### 3.1. Microcontroller

*   **Component:** ESP32-WROOM-32 Module
*   **Reasoning:** Chosen for its processing power, integrated wireless capabilities, and critical deep sleep modes for conserving backup battery life.

### 3.2. Valve Actuators

*   **Component:** 12V Latching Solenoid Valves (x3)
*   **Reasoning:** Latching solenoids are essential for efficiency, requiring only a short energy pulse to change state.

### 3.3. H-Bridge Driver

*   **Component:** TI DRV8871 H-Bridge Driver (x3)
*   **Reasoning:** A robust driver with a wide operating voltage, ideal for 12V valves. Three individual drivers provide excellent electrical isolation.

### 3.4. Water Flow Sensor

*   **Component:** YF-S201 Hall Effect Water Flow Sensor
*   **Reasoning:** A common and cost-effective sensor that can be monitored efficiently by the ESP32's pulse counter.

### 3.5. Temperature Sensor

*   **Component:** DS18B20 Waterproof Temperature Sensor
*   **Reasoning:** Its 1-Wire interface simplifies wiring, and the waterproof package is suitable for the application environment.

### 3.6. Power Subsystem

*   **Primary Power Source:** 5V DC via a waterproof USB-C connector.
*   **Backup Power Source:** 9V Alkaline Battery (e.g., 6LR61).
*   **Power Switching:** An automatic power multiplexing circuit will prioritize the 5V USB input. If USB power is lost, it will automatically switch to the 9V battery backup.
*   **Voltage Regulation:**
    *   **For ESP32:** A high-efficiency, 3.3V LDO regulator will supply power to the microcontroller, drawing from the selected 5V or 9V source.
    *   **For Valves:** A single **DC-DC Boost Converter** (e.g., based on MT3608 or XL6009) will be used. This converter will take the selected input power (either 5V from USB or 9V from the battery) and step it up to a stable 12V. Its enable pin will be controlled by the ESP32 to ensure it is only active during valve actuation.

#### 3.6.1. Power Switching Circuit Detail

The circuit is designed to prioritize USB power and automatically failover to the battery. The +5V USB line splits to perform two jobs: provide main power and control the battery switch (a P-channel MOSFET).

**Corrected Schematic:**
```
                                     +9V Battery (+)
                                          |
                                        -----
                                       |     | S (Source)
                                       |  Q1 | P-Channel MOSFET
                                       |     | G (Gate)----(POINT C)
                                        -----   D (Drain)
                                          |
                                          |
      +-----------------------------------(POINT B)------> V_IN to System
      |
      |
(POINT A)----[Anode]  (D1, Schottky Diode)  [Cathode]----(POINT B)
      |
      |
+5V USB (+)----(POINT A)----(Wire Split)----(POINT C)
                                                 |
                                                ---
                                                | | R1 (Pull-down Resistor)
                                                ---
                                                 |
                                                GND
```

**Connection & Operation:**

1.  **USB Power Input (POINT A):** The +5V from the USB port enters the circuit here and the line splits.

2.  **Path 1 (Powering the System):**
    *   From **POINT A**, power flows through the Schottky Diode (D1) to **POINT B**.
    *   **POINT B** becomes the main power rail for the device, feeding the 3.3V LDO and the 12V boost converter.
    *   The diode (D1) acts as a one-way valve, preventing battery voltage from ever flowing back to the USB port.

3.  **Path 2 (Controlling the Battery Switch):**
    *   A second wire runs from **POINT A** to **POINT C**.
    *   **POINT C** connects to the Gate of the MOSFET (Q1). When 5V is present here, it turns the MOSFET **OFF**, disconnecting the battery.
    *   When USB power is removed, **POINT C** is pulled to 0V by the resistor (R1), which turns the MOSFET **ON**, allowing the battery to power the circuit via **POINT B**.

**Recommended Components:**
*   **P-Channel MOSFET (Q1):** AO3401A (SMD) or FQP27P06 (Through-Hole).
*   **Schottky Diode (D1):** 1N5817 or 1N5819.
*   **Alternative IC:** For a more compact solution, a dedicated Power MUX IC like the Texas Instruments **TPS211xA** series can be used.

## 4. Pinout and Wiring

| ESP32 GPIO | Function                   | Connected To                | Notes                                     |
| ---------- | -------------------------- | --------------------------- | ----------------------------------------- |
| GPIO14     | Valve 1 Input 1            | DRV8871 #1 - PH             | H-Bridge Input 1 for Valve 1              |
| GPIO12     | Valve 1 Input 2            | DRV8871 #1 - EN             | H-Bridge Input 2 for Valve 1              |
| GPIO16     | Valve 2 Input 1            | DRV8871 #2 - PH             | H-Bridge Input 1 for Valve 2              |
| GPIO17     | Valve 2 Input 2            | DRV8871 #2 - EN             | H-Bridge Input 2 for Valve 2              |
| GPIO25     | Valve 3 Input 1            | DRV8871 #3 - PH             | H-Bridge Input 1 for Valve 3              |
| GPIO26     | Valve 3 Input 2            | DRV8871 #3 - EN             | H-Bridge Input 2 for Valve 3              |
| GPIO32     | 12V Boost Converter Enable | Boost Converter EN Pin      | Activates the 12V supply for valves       |
| GPIO13     | Water Flow Sensor Pulse    | YF-S201 Signal Pin          | Utilizes ESP32 Pulse Counter              |
| GPIO15     | Temp Sensor (1-Wire)       | DS18B20 Data Pin            | Requires a 4.7kΩ pull-up resistor to 3.3V |
| GPIO2      | Status LED                 | On-board LED                | For visual feedback                       |
| GPIO36     | 9V Battery Voltage Sense   | Voltage Divider from 9V Bat | For backup battery level monitoring       |
| GPIO39     | DHT22 sensor               | DHT22 Data pin              | Temperature and Humidity sensor           |
| GPIO27     | Enable 12V                 | 12V Boost Converter         | Enable 12V for H-Bridge drivers           |

## 5. Power Consumption Analysis

Efficiency is paramount for maximizing the life of the 9V backup battery.

*   **Deep Sleep:** The ESP32 will remain in deep sleep mode (~10µA) for over 99% of the time when on battery backup.
*   **Active Mode (on Battery):** During a watering cycle, the ESP32 will wake up, enable the 12V boost converter, send the ~150ms pulse to the valve, disable the converter, publish a status message, and return to deep sleep.
*   **Estimated Backup Life:** A standard 9V alkaline battery (~500mAh) can support the system for several weeks to months of backup operation.

## 6. Enclosure

*   **Material:** UV-resistant ABS or Polycarbonate.
*   **Rating:** IP67-rated.
*   **Connectors:** Waterproof M8/M12 connectors for valves/sensors and a waterproof USB-C connector for power.
