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
    pinMode(PIN_VCU_RESET, OUTPUT);
    
    // Set Initial Values
    digitalWrite(PIN_APPS1_SIM, LOW);
    digitalWrite(PIN_APPS2_SIM, LOW);
    digitalWrite(PIN_BSE_SIM, LOW);
    digitalWrite(PIN_TS_ACTIVE_SIM, LOW);
    digitalWrite(PIN_DRIVER_ACTION_SIM, LOW);
    digitalWrite(PIN_VCU_RESET, LOW);
}

void resetVCU() {
    digitalWrite(PIN_VCU_RESET, LOW);
    delay(100);
    digitalWrite(PIN_VCU_RESET, HIGH);
    delay(100);
    digitalWrite(PIN_VCU_RESET, LOW);
}

void setupSerialBlocking() {
    Serial.begin(SERIAL_BAUDRATE);
    while (!Serial) {  }
}

void endSerialBlocking() {
    Serial.end();
    while (Serial) { }
}
