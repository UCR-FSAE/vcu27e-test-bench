#include "util.hpp"

void setupPins() {
    // Set Input Pins
    pinMode(PIN_BRAKE_LIGHT_MONITOR, INPUT);
    pinMode(PIN_BUZZER_MONITOR, INPUT);
    
    // Set Output Pins
    pinMode(PIN_APPS1_SIM, OUTPUT);
    pinMode(PIN_APPS2_SIM, OUTPUT);
    pinMode(PIN_BSE_SIM, OUTPUT);
    pinMode(PIN_TS_ACTIVE_SIM, OUTPUT);
    pinMode(PIN_DRIVER_ACTION_SIM, OUTPUT);
    
    // Set Initial Values
    digitalWrite(PIN_APPS1_SIM, LOW);
    digitalWrite(PIN_APPS2_SIM, LOW);
    digitalWrite(PIN_BSE_SIM, LOW);
    digitalWrite(PIN_TS_ACTIVE_SIM, LOW);
    digitalWrite(PIN_DRIVER_ACTION_SIM, LOW);
}

void setupSerialBlocking() {
    SERIAL_PORT_MONITOR.begin(SERIAL_BAUDRATE);
    while (!SERIAL_PORT_MONITOR) {  }
}
