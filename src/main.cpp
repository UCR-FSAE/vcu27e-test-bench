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
    initSerialBlocking();
    initIO();
    
    if (CAN_OK == CAN.begin(CAN_500KBPS, MCP_8MHz)) {
        canOk = 1;
        SERIAL_PORT_MONITOR.println("CAN init passed");
    } else {
        SERIAL_PORT_MONITOR.println("CAN init fail");
    }
    
    SERIAL_PORT_MONITOR.println("Initialization Done.");
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

    SERIAL_PORT_MONITOR.print("time: ");
    SERIAL_PORT_MONITOR.print(lastPrintMs);
    SERIAL_PORT_MONITOR.println(" ms");

    SERIAL_PORT_MONITOR.print("  SENT driver=");
    SERIAL_PORT_MONITOR.print(driverHigh ? "HIGH" : "LOW");
    SERIAL_PORT_MONITOR.print(" ts=");
    SERIAL_PORT_MONITOR.print(tsHigh ? "HIGH" : "LOW");
    SERIAL_PORT_MONITOR.print(" apps1=");
    SERIAL_PORT_MONITOR.print(apps1High ? "HIGH" : "LOW");
    SERIAL_PORT_MONITOR.print(" apps2=");
    SERIAL_PORT_MONITOR.print(apps2High ? "HIGH" : "LOW");
    SERIAL_PORT_MONITOR.print(" bse=");
    SERIAL_PORT_MONITOR.print(bseHigh ? "HIGH" : "LOW");
    SERIAL_PORT_MONITOR.println(" | CAN tx none");

    SERIAL_PORT_MONITOR.print("  RECV brake=");
    SERIAL_PORT_MONITOR.print(brakeV, 2);
    SERIAL_PORT_MONITOR.print("V buzzer=");
    SERIAL_PORT_MONITOR.print(buzzerV, 2);
    SERIAL_PORT_MONITOR.print("V");
    SERIAL_PORT_MONITOR.print(" | CAN rx ");
    if (frameReady) { 
        SERIAL_PORT_MONITOR.print("0x0C0 =");
        for (unsigned char i = 0; i < 8; i++) {
            SERIAL_PORT_MONITOR.print(" ");
            if (lastFrame[i] < 16) {
                SERIAL_PORT_MONITOR.print("0");
            }
            SERIAL_PORT_MONITOR.print(lastFrame[i], HEX);
        }
        // bytes 0 and 1 are little-endian torque, byte 5 is enable inverter
        unsigned int torque = lastFrame[0] | ((unsigned int)lastFrame[1] << 8);
        SERIAL_PORT_MONITOR.print(" torque=");
        SERIAL_PORT_MONITOR.print(torque);
        frameReady = 0;
    } else {
        SERIAL_PORT_MONITOR.print("none");
    }
    SERIAL_PORT_MONITOR.println();
}
