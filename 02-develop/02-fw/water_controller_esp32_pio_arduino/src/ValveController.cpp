/**
 * @file ValveController.cpp
 * @brief Implements the ValveController class for controlling water valves.
 * 
 * This file contains the implementation of the ValveController class, which provides
 * a high-level interface for controlling the latching solenoid valves. It abstracts
 * the low-level H-bridge control logic, allowing other parts of the firmware to
 * simply call openValve() and closeValve() without needing to know the details
 * of the hardware implementation. This encapsulation is a key principle of good
 * object-oriented design and makes the code cleaner and more maintainable.
 * 
 * @author ofirbd
 * @version 1.0
 * @date 2025-07-04
 */
#include "ValveController.h"
#include "Config.h"

ValveController::ValveController() {
    // Constructor
}

void ValveController::begin() {
    // Set all valve control pins to output
    pinMode(VALVE1_PIN_A, OUTPUT);
    pinMode(VALVE1_PIN_B, OUTPUT);
    pinMode(VALVE2_PIN_A, OUTPUT);
    pinMode(VALVE2_PIN_B, OUTPUT);
    pinMode(VALVE3_PIN_A, OUTPUT);
    pinMode(VALVE3_PIN_B, OUTPUT);

    // Ensure all pins are low initially
    digitalWrite(VALVE1_PIN_A, LOW);
    digitalWrite(VALVE1_PIN_B, LOW);
    digitalWrite(VALVE2_PIN_A, LOW);
    digitalWrite(VALVE2_PIN_B, LOW);
    digitalWrite(VALVE3_PIN_A, LOW);
    digitalWrite(VALVE3_PIN_B, LOW);
}

void ValveController::openValve(int valveNumber) {
    Serial.printf("Opening valve %d\n", valveNumber);
    switch (valveNumber) {
        case 1:
            sendPulse(VALVE1_PIN_A, VALVE1_PIN_B);
            break;
        case 2:
            sendPulse(VALVE2_PIN_A, VALVE2_PIN_B);
            break;
        case 3:
            sendPulse(VALVE3_PIN_A, VALVE3_PIN_B);
            break;
    }
}

void ValveController::closeValve(int valveNumber) {
    Serial.printf("Closing valve %d\n", valveNumber);
    switch (valveNumber) {
        case 1:
            // Reverse polarity to close
            sendPulse(VALVE1_PIN_B, VALVE1_PIN_A);
            break;
        case 2:
            sendPulse(VALVE2_PIN_B, VALVE2_PIN_A);
            break;
        case 3:
            sendPulse(VALVE3_PIN_B, VALVE3_PIN_A);
            break;
    }
}

// Private method to send the physical pulse
void ValveController::sendPulse(int highPin, int lowPin) {
    digitalWrite(highPin, HIGH);
    digitalWrite(lowPin, LOW);
    delay(VALVE_PULSE_MS);
    digitalWrite(highPin, LOW);
    // lowPin is already LOW
}
