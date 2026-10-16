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

// Sim Output Variables
int apps1High = 0;
int apps2High = 0;
int bseHigh = 0;
int driverHigh = 0; // LOW(0) means pressed
int tsHigh = 0;

// Loop Variables
unsigned long lastRunMS = 0;
CANMessage lastMsg;

void setup() {
    setupSerialBlocking();
    setupPins();
    setupCANBlocking();
    resetVCU();
    
    // Init Check
    
    Serial.println("Waiting for inverter initialization sequence...");
    
    waitForMsg(lastMsg);
    
    Serial.print(lastMsg.id, HEX);
    Serial.print(" = ");
    for (unsigned char i = 0; i < 8; i++) {
        Serial.print(" ");
        Serial.print(lastMsg.data[i], HEX);
    }
    Serial.println("");
    
    if (lastMsg.id != CAN_ID_VCU_TO_INVERTER || lastMsg.length != 8) {
        Serial.println("Missed Disable CMD");
        return;
    }
    
    for (unsigned char i = 0; i < 8; i++) {
        if (lastMsg.data[i] != 0x00) {
            Serial.println("Missed Disable CMD");
            return;
        }
    }
    
    clearMsg(lastMsg);
    
    waitForMsg(lastMsg);
    
    Serial.print(lastMsg.id);
    Serial.print(" = ");
    for (unsigned char i = 0; i < 8; i++) {
        Serial.print(" ");
        Serial.print(lastMsg.data[i], HEX);
    }
    
    if (lastMsg.id != CAN_ID_VCU_TO_INVERTER || lastMsg.length != 8) {
        Serial.println("Missed Enable CMD");
        return;
    }
    
    for (unsigned char i = 0; i < 8; i++) {
        if (i != 4 || i != 5) {
            if (lastMsg.data[i] != 0x00) {
                Serial.println("Missed Enable CMD");
                return;
            }
        } else {
            if (lastMsg.data[i] != 0x01) {
                Serial.println("Missed Enable CMD");
                return;
            }
        }
    }
    
    const byte data[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00};
    CAN.sendMsgBuf(CAN_ID_INVERTER_TO_VCU, 0x00, 0x00, 0x08, data);
    Serial.println("Sent Status Message.");
    
    Serial.println("Inverter Initialization Successful.");
}

void loop() {
    // -------------- CAN frame reading --------------
    // Read CAN frame 0x0C0 (CAN_ID_VCU_TO_INVERTER): stays 0 if not connected    
    if (getMsg(lastMsg)) {
        if (lastMsg.id != CAN_ID_VCU_TO_INVERTER) clearMsg(lastMsg);
    }
    
    // -------------- Print output every 0.25 s--------------
    if (millis() - lastRunMS < 250) { return; }
    lastRunMS = millis();
    
    /* Expected output with vcu not connected:
    time: 250 ms
        SENT driver=HIGH ts=LOW apps1=LOW apps2=LOW bse=LOW | CAN tx none
        RECV brake=LOW buzzer=LOW | CAN rx none
    with a frame: CAN rx 0x0C0 = A0 0F 00 00 00 01 00 00 torque=4000
    */
    // analogRead is 0 at 0 V and 1023 at 5 V.
    float brakeV = analogRead(PIN_BRAKE_LIGHT_MONITOR) * (5.0f / 1023.0f);
    float buzzerV = analogRead(PIN_BUZZER_MONITOR) * (5.0f / 1023.0f);

    Serial.print("time: ");
    Serial.print(lastRunMS);
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
    if (lastMsg.valid) {
        Serial.print(lastMsg.id, HEX);
        Serial.print(" = ");
        
        for (unsigned char i = 0; i < 8; i++) {
            Serial.print(" ");
            if (lastMsg.data[i] < 16) {
                Serial.print(lastMsg.data[i], DEC);
            } else {
                Serial.print("0x");
                Serial.print(lastMsg.data[i], HEX);
            }
        }
        
        // bytes 0 and 1 are little-endian torque, byte 5 is enable inverter
        unsigned int torque =
            lastMsg.data[0] | ((unsigned int)lastMsg.data[1] << 8);
        Serial.print(" torque=");
        Serial.print(torque);
        clearMsg(lastMsg);
    } else {
        Serial.print("none");
    }
    Serial.println();
}
