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
