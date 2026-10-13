/* This code is taken from the CANLogger repo (https://github.com/UCR-FSAE/CANLogger).
* Thanks Justin!
*
* Author: Emad Saadat and Emily Tan (and Justin Im).
*/

#include <Arduino.h>
#include <SPI.h>
#include "mcp2515_can.h"

// ------------- Variable Declarations -------------
// CAN SHIELD CONFIGURATION
const uint8_t CAN_CS_PIN = 9;
const auto CAN_BAUDRATE = 500000;

// ------------- CAN ids (inverter.h) -------------
const unsigned long CAN_ID_VCU_TO_INVERTER = 0x0C0; // Inverter_INVERTER_COMMAND_ID
const unsigned long CAN_ID_INVERTER_CLEAR = 0x0C1;
const unsigned long CAN_ID_INVERTER_TO_VCU = 0x0AA; // Inverter_INVERTER_STATUS_ID

// Inverter_TransmitCANMessage()
const uint8_t CAN_COMMAND_LENGTH = 8;
const uint8_t INVERTER_DIRECTION_FORWARD = 0x00;
const uint8_t INVERTER_DIRECTION_REVERSE = 0x01;
const uint8_t INVERTER_ENABLE = 0x01;
const uint8_t INVERTER_DISABLE = 0x00;
// InverterCheck() bit 0 of byte 6
const uint8_t INVERTER_STATUS_READY = 0x01;

// ------------- Timing from the firmware and the rules -------------
const unsigned long INVERTER_INIT_GAP_MS = 500;       // HAL_Delay between disable and enable
const unsigned long BUZZER_TIME_MS = 1500;            // RTDCheck()
const unsigned long BUZZER_TIME_MIN_MS = 1000;
const unsigned long BUZZER_TIME_MAX_MS = 3000;
const unsigned long CAN_FRAME_PERIOD_MS = 30;         // HAL_Delay at the bottom of the loop
const unsigned long APPS_IMPLAUSIBILITY_TIME_MS = 100;

// ------------- Pedal and brake calibration (util.c, main.h) -------------
const float APPS1_MIN_V = 1.4f;
const float APPS1_MAX_V = 3.2f;
const float APPS2_MIN_V = 0.4f;
const float APPS2_MAX_V = 1.4f;
const float BSE_MIN_V = 0.3f;
const float BSE_MAX_V = 1.3f;
const float BRAKES_ACTIVATION_THRESHOLD_V = 0.5f;
const float APPS_IMPLAUSIBILITY_THRESHOLD_PERCENT = 20.0f;
const float APPS_RANGE_LOW_PERCENT = 0.0f;
const float APPS_RANGE_HIGH_PERCENT = 110.0f;
const float DEADZONE_PERCENT = 15.0f;
const float MAX_TORQUE_COMMAND = 4000.0f;

// ------------- PEDAL TEST POINTS -------------
// TODO: Confirm Pedal test values
const uint8_t PEDAL_TEST_VALUES[] = {
    0, 10, 25, 50, 75, 100
};

// ------------- PIN CONFIGURATION -------------
const int PIN_APPS1_SIM = 3;
const int PIN_APPS2_SIM = 5;
const int PIN_BSE_SIM = 6;
const int PIN_DRIVER_ACTION_SIM = 7;
const int PIN_TS_ACTIVE_SIM = 8; 

int apps1High = 0;
int apps2High = 0;
int bseHigh = 0;
int driverHigh = 0; // LOW(0) means pressed
int tsHigh = 0;

const int PIN_BRAKE_LIGHT_MONITOR = A0;
const int PIN_BUZZER_MONITOR = A1;

// ------------- CAN controller -------------
mcp2515_can CAN(CAN_CS_PIN);

// ------------- TIMING VARIABLES -------------
unsigned long lastPrintMs = 0;
int canOk = 0;
int frameReady = 0;
unsigned char lastFrame[8] = {0, 0, 0, 0, 0, 0, 0, 0};

void setup() {
    // --------------- Initialize serial output ---------------
    SERIAL_PORT_MONITOR.begin(CAN_BAUDRATE);
    while (!SERIAL_PORT_MONITOR) {  }

    // --------------- Initialize GPIO pins ---------------
    // set up to output to vcu
    pinMode(PIN_APPS1_SIM, OUTPUT);
    pinMode(PIN_APPS2_SIM, OUTPUT);
    pinMode(PIN_BSE_SIM, OUTPUT);
    pinMode(PIN_TS_ACTIVE_SIM, OUTPUT);
    pinMode(PIN_DRIVER_ACTION_SIM, OUTPUT);
    // set initial values
    digitalWrite(PIN_APPS1_SIM, LOW);
    digitalWrite(PIN_APPS2_SIM, LOW);
    digitalWrite(PIN_BSE_SIM, LOW);
    digitalWrite(PIN_TS_ACTIVE_SIM, LOW);
    digitalWrite(PIN_DRIVER_ACTION_SIM, LOW);
    // set up to input from vcu
    pinMode(PIN_BRAKE_LIGHT_MONITOR, INPUT);
    pinMode(PIN_BUZZER_MONITOR, INPUT);

// --------------- CAN initialization ---------------
    if (CAN_OK == CAN.begin(CAN_500KBPS, MCP_8MHz)) {
        canOk = 1;
        SERIAL_PORT_MONITOR.println("CAN init passed");
    } else {
        SERIAL_PORT_MONITOR.println("CAN init fail");
        // continue even if CAN init fails
    }
    SERIAL_PORT_MONITOR.println("Starting...");
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
        // if RTD sequence is enabled, the 6th byte will alternate to 1 or 0 every 500 ms
        for (unsigned char i = 0; i < 8; i++) {
            SERIAL_PORT_MONITOR.print(" ");
            SERIAL_PORT_MONITOR.print(lastFrame[i]);
        }
        frameReady = 0;
    } else {
        SERIAL_PORT_MONITOR.print("none");
    }
    SERIAL_PORT_MONITOR.println();
}
