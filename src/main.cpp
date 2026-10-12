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
const auto CAN_BITRATE = CAN_500KBPS;

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

// ------------- VCU INPUT / OUTPUT DATA STRUCTURES -------------
struct Inputs {
    float apps1_raw = 0.0f;
    float apps2_raw = 0.0f;
    float bse_raw = 0.0f;
    float apps_percentage = 0.0f;

    bool ts_active = false;
    bool driver_act = false;
    bool inverter_enabled = false;
    bool brake_pressed = false;
    bool pedal_fault = false;
};
struct Outputs {
    float torque_cmd = 0.0f;
    bool inverter_enable = false;
    bool buzzer = false;
    bool brake_light = false;
};

// ------------- ARDUINO TEST-BENCH SIGNAL PINS -------------
// TODO: Finalize these
const int PIN_DRIVER_ACTION = 0;
const int PIN_TS_ACTIVE =  0;
const int PIN_APPS1_SIM = 0;
const int PIN_APPS2_SIM = 0;
const int PIN_BSE_SIM = 0;
const int PIN_BRAKE_LIGHT_MONITOR = 0;
const int PIN_BUZZER_MONITOR = 0;

// ------------- Test results -------------
int passCount = 0;
int failCount = 0;

// ------------- CAN controller -------------
mcp2515_can CAN(CAN_CS_PIN);

void setup() {
    // --------------- Initialize serial output ---------------
    SERIAL_PORT_MONITOR.begin(CAN_BITRATE);
    while (!SERIAL_PORT_MONITOR) {  }

    // --------------- Initialize GPIO pins ---------------
    //TODO: initialize GPIO pins

    // --------------- Initialize CAN controller ---------------
    while (CAN_OK != CAN.begin(CAN_BITRATE)) {
        SERIAL_PORT_MONITOR.println("CAN init fail, retrying...");
        delay(100);
    }
    SERIAL_PORT_MONITOR.println("CAN init ok!");

    // --------------- Test sequence ---------------
    SERIAL_PORT_MONITOR.println("Starting...");

    // --------------- Configure initial pedal and brake inputs ---------------

    // --------------- Verify initial VCU state ---------------

    // --------------- Simulate required RTD sequence ---------------

    // --------------- Verify VCU enters inverter initialization state ---------------

    // --------------- Perform inverter handshake ---------------

    // --------------- Verify VCU enters expected state ---------------

    // --------------- For each predefined pedal test value ---------------
    // --------------- Set simulated APPS inputs ---------------
    // --------------- Send or await scheduled CAN messages ---------------
    // --------------- Read VCU response ---------------
    // --------------- Verify expected torque/state response ---------------
    // --------------- Print PASS or FAIL ---------------

    // add more tests in the future...
}

void loop() {
}
