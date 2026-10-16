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
bool init_success = false;

enum inverter_state { ENABLED, DISABLED, STARTUP };
inverter_state inv_state;

void setup() {
    inv_state = STARTUP;
    setupSerialBlocking();
    setupPins();
    setupCANBlocking();
    resetVCU();
    
    // Init Check
    
    while (!init_success) {
        Serial.println("Waiting for inverter initialization sequence...");
        
        waitForMsg(lastMsg);
        
        Serial.print(lastMsg.id, HEX);
        Serial.print(" = ");
        for (unsigned char i = 0; i < 8; i++) {
            Serial.print(" ");
            Serial.print(lastMsg.data[i], HEX);
        }
        Serial.println("");
        
        if (inv_state == STARTUP) {
            if (lastMsg.id != CAN_ID_VCU_TO_INVERTER || lastMsg.length != 8) {
                Serial.println("Missed Disable CMD");
            } else {
                for (unsigned char i = 0; i < 8; i++) {
                    if (lastMsg.data[i] != 0x00) {
                        Serial.println("Missed Disable CMD");
                    } else { inv_state = DISABLED; }
                }
            }
        } else if (inv_state == DISABLED) {
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
    if (millis() - lastRunMS < 100) { return; }
    lastRunMS = millis();
    
    const byte statusData[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00};
    CAN.trySendMsgBuf(CAN_ID_INVERTER_TO_VCU, 0x00, 0x00, 0x08, statusData);
}
