/**
 * @file LedControl.h
 * @brief Controls the status LED.
 * @author ofirbd
 * @version 1.0
 * @date 2025-07-04
 */
#ifndef LED_CONTROL_H
#define LED_CONTROL_H

#include <Arduino.h>

class LedControl {
public:
    LedControl(int pin);
    void begin();
    void on();
    void off();
    void blink(unsigned long interval);

private:
    int _pin;
    int _status; // Current state of the LED (LOW or HIGH)
    unsigned long _previousMillis; // Last time LED state was updated
};

#endif // LED_CONTROL_H
