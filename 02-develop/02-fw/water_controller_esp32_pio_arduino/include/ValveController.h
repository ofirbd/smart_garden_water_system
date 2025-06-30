#ifndef VALVE_CONTROLLER_H
#define VALVE_CONTROLLER_H

#include <Arduino.h>

class ValveController {
public:
    ValveController();
    void begin();
    void openValve(int valveNumber);
    void closeValve(int valveNumber);

private:
    void sendPulse(int pinA, int pinB);
};

#endif // VALVE_CONTROLLER_H
