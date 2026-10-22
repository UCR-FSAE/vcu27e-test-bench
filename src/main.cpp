/*
* This code is taken from the CANLogger repo (https://github.com/UCR-FSAE/CANLogger).
* Thanks Justin!
*
* Author: Emad Saadat and Emily Tan
*/

#include <Arduino.h>
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
bool init_success = false;
bool inverter_disabled_cmd = false;
bool inverter_enabled_cmd = false;

enum inverter_state { ENABLED, DISABLED, STARTUP };
inverter_state inv_state;

const byte statusBuf[8] = {0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00};

void setup() {
    setupSerialBlocking();
    setupPins();
    setupCANBlocking();
    resetVCU();
    
    // Init Check
    
    while (!init_success) {
        if (inv_state == STARTUP) {
            Serial.println("Waiting for inverter initialization sequence...");
        }
        
        waitForMsg(lastMsg);
        
        Serial.print(lastMsg.id, HEX);
        Serial.print(" = ");
        for (unsigned char i = 0; i < 8; i++) {
            Serial.print(" ");
            Serial.print(lastMsg.data[i], HEX);
        }
        Serial.println("");
        
        if (lastMsg.valid && inv_state == STARTUP) {
            if (lastMsg.id != CAN_ID_VCU_TO_INVERTER || lastMsg.length != 8) {
                Serial.println("Missed Disable CMD");
            } else {
                for (unsigned char i = 0; i < 8; i++) {
                    if (lastMsg.data[i] != 0x00) {
                        Serial.println("Missed Disable CMD");
                    } else {
                        inverter_disabled_cmd = true;
                    }
                }
                if (inverter_disabled_cmd) { 
                    inv_state = DISABLED; 
                    Serial.println("Inverter disabled");
                }
            }
        } else if (inv_state == DISABLED) {
            Serial.println("Sending inverter status in 1.5 s");
            delay(1500);
            CAN.trySendMsgBuf(CAN_ID_INVERTER_TO_VCU, 0x00, 0x00, 0x08, statusBuf); 
            delay(15);   
            if (lastMsg.id != CAN_ID_VCU_TO_INVERTER || lastMsg.length != 8) {
                Serial.println("Missed Enable CMD");
            } else {
                for (unsigned char i = 0; i < 8; i++) {
                    if (i != 4 && i != 5) {
                        if (lastMsg.data[i] != 0x00) {
                            Serial.println("Incorrect Enable CMD");
                        }
                    }
                }

                if (lastMsg.data[4] != 0x00 || lastMsg.data[5] != 0x01) {
                    Serial.println("Incorrect Enable CMD");
                } else {
                    inv_state = ENABLED;
                    init_success = true;
                }
            }
        }
        clearMsg(lastMsg);
        delay(10);
    }
    
    Serial.println("Inverter Initialization Successful.");
}

void loop() {
    static unsigned long lastRunMS = 0;
    static CANMessage lastMsg;
    
    if (millis() - lastRunMS < 10) { return; }
    lastRunMS = millis();
    
    if (getMsg(lastMsg)) printMsg(lastMsg);
}