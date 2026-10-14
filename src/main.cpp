/*
* This code is taken from the CANLogger repo (https://github.com/UCR-FSAE/CANLogger).
* Thanks Justin!
*
* Author: Emad Saadat and Emily Tan
*/

#include <Arduino.h>
#include "constants.hpp"
#include "can.hpp"
#include "util.hpp"

int apps1High = 0;
int apps2High = 0;
int bseHigh = 0;
int driverHigh = 0; // LOW(0) means pressed
int tsHigh = 0;

// ------------- TIMING VARIABLES -------------
unsigned long lastPrintMs = 0;
int canOk = 0;
int frameReady = 0;
unsigned char lastFrame[8] = {0, 0, 0, 0, 0, 0, 0, 0};

void setup() {
    setupSerialBlocking();
    setupPins();
    setupCANBlocking();
    Serial.println("Initialization Done.");
}

void loop() {
    // -------------- CAN frame reading --------------
    // Read CAN frame 0x0C0 (CAN_ID_VCU_TO_INVERTER): stays 0 if not connected
    if (canOk && CAN.checkReceive() == CAN_MSGAVAIL) {
        unsigned long id = 0;
        unsigned char len = 0;
        unsigned char buf[8] = {0, 0, 0, 0, 0, 0, 0, 0};
        if (CAN.readMsgBufID(&id, &len, buf) == CAN_OK && id == CAN_ID_VCU_TO_INVERTER) {
            for (unsigned char i = 0; i < 8; i++) {
                lastFrame[i] = (i < len) ? buf[i] : 0;
            }
            frameReady = 1;
        }
    }

    // -------------- Print output every 0.25 s--------------
    if (millis() - lastPrintMs < 250) {
        return;
    }
    /* Expected output with vcu not connected:
    time: 250 ms
        SENT driver=HIGH ts=LOW apps1=LOW apps2=LOW bse=LOW | CAN tx none
        RECV brake=LOW buzzer=LOW | CAN rx none
    with a frame: CAN rx 0x0C0 = A0 0F 00 00 00 01 00 00 torque=4000
    */

    lastPrintMs = millis();
    // analogRead is 0 at 0 V and 1023 at 5 V.
    float brakeV = analogRead(PIN_BRAKE_LIGHT_MONITOR) * (5.0f / 1023.0f);
    float buzzerV = analogRead(PIN_BUZZER_MONITOR) * (5.0f / 1023.0f);

    Serial.print("time: ");
    Serial.print(lastPrintMs);
    Serial.println(" ms");

    Serial.print("  SENT driver=");
    Serial.print(driverHigh ? "HIGH" : "LOW");
    Serial.print(" ts=");
    Serial.print(tsHigh ? "HIGH" : "LOW");
    Serial.print(" apps1=");
    Serial.print(apps1High ? "HIGH" : "LOW");
    Serial.print(" apps2=");
    Serial.print(apps2High ? "HIGH" : "LOW");
    Serial.print(" bse=");
    Serial.print(bseHigh ? "HIGH" : "LOW");
    Serial.println(" | CAN tx none");

    Serial.print("  RECV brake=");
    Serial.print(brakeV, 2);
    Serial.print("V buzzer=");
    Serial.print(buzzerV, 2);
    Serial.print("V");
    Serial.print(" | CAN rx ");
    if (frameReady) { 
        Serial.print("0x0C0 =");
        for (unsigned char i = 0; i < 8; i++) {
            Serial.print(" ");
            if (lastFrame[i] < 16) {
                Serial.print("0");
            }
            Serial.print(lastFrame[i], HEX);
        }
        // bytes 0 and 1 are little-endian torque, byte 5 is enable inverter
        unsigned int torque = lastFrame[0] | ((unsigned int)lastFrame[1] << 8);
        Serial.print(" torque=");
        Serial.print(torque);
        frameReady = 0;
    } else {
        Serial.print("none");
    }
    Serial.println();
}
