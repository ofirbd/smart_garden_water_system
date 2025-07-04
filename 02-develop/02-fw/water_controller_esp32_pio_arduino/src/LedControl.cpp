/**
 * @file LedControl.cpp
 * @brief Implements the LedControl class for managing the status LED.
 * 
 * This file contains the implementation of the LedControl class, which provides
 * functionality to control an LED for status indication. It allows turning the
 * LED on, off, and blinking at a specified interval. This class follows
 * best practices by encapsulating the LED control logic, making it reusable
 * and easy to manage. The use of millis() for blinking avoids blocking the
 * main loop, which is crucial for responsive embedded systems.
 * 
 * @author ofirbd
 * @version 1.0
 * @date 2025-07-04
 */
#include "LedControl.h"

LedControl::LedControl(int pin) : _pin(pin), _status(LOW), _previousMillis(0) {
}

void LedControl::begin() {
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, _status); // Ensure initial state is off
}

void LedControl::on() {
    digitalWrite(_pin, HIGH);
    _status = HIGH;
}

void LedControl::off() {
    digitalWrite(_pin, LOW);
    _status = LOW;
}

void LedControl::blink(unsigned long interval) {
    unsigned long currentMillis = millis();

    if (currentMillis - _previousMillis >= interval) {
        _previousMillis = currentMillis;

        if (_status == LOW) {
            on();
        } else {
            off();
        }
    }
}
