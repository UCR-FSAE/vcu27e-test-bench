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

// CAN MESSAGE IDS
// TODO: Get these from the VCU
// These zero values are placeholders
const unsigned long CAN_ID_INVERTER_TO_VCU = 0x000;
const unsigned long CAN_ID_VCU_TO_INVERTER = 0x000;
const unsigned long CAN_ID_VCU_STATE = 0x000;
const unsigned long CAN_ID_TORQUE_COMMAND = 0x000;

// VCU STATE ENCODINGS
// TODO: Get these from the VCU
const uint8_t STATE_NOT_READY = 0xFF;
const uint8_t STATE_RTD = 0xFF;
const uint8_t STATE_INVERTER_INIT = 0xFF;
const uint8_t STATE_DRIVE = 0xFF;
const uint8_t STATE_FAULT = 0xFF;

// TIMEOUTS AND TIMING REQUIREMENTS
const unsigned long CAN_LOSS_TIMEOUT_MS = 100;
// TODO: Get these from the rules or VCU implementation
const unsigned long INVERTER_TIMEOUT_MS = 0;
const unsigned long BUZZER_TIME_MIN_MS = 1000;
const unsigned long BUZZER_TIME_MAX_MS = 3000;
const unsigned long CAN_FRAME_PERIOD_MS = 0;
const unsigned long CAN_FRAME_TOLERANCE_MS = 0;
// TODO: Test-bench response wait
const unsigned long CAN_RESPONSE_TIMEOUT_MS = 000;

// APPS VOLTAGE BOUNDS
const float APPS1_MIN_V = 1.4f;
const float APPS1_MAX_V = 3.2f;

const float APPS2_MIN_V = 0.4f;
const float APPS2_MAX_V = 1.4f;

// BSE VOLTAGE BOUNDS
const float BSE_MIN_V = 0.3f;
const float BSE_MAX_V = 1.3f;

// APPS PLAUSIBILITY REQUIREMENTS
// VCU calculates the APPS mismatch as a 10% mismatch lasting more than 100 ms
const float APPS_IMPLAUSIBILITY_THRESHOLD_PERCENT = 10.0f;
const unsigned long APPS_IMPLAUSIBILITY_TIME_MS = 100;

// BRAKE-THROTTLE INTERLOCK
const float BRAKE_THROTTLE_THRESHOLD_PERCENT = 25.0f;

// PEDAL TEST POINTS
// TODO: Confirm Pedal test values
const uint8_t PEDAL_TEST_VALUES[] = {
    0, 10, 25, 50, 75, 100
};

// INVERTER STATUS ENCODINGS
// TODO: get actual protocol values
const uint8_t INVERTER_STATUS_READY = 0xFF;
const uint8_t INVERTER_STATUS_FAULT = 0xFF;

// VCU INPUT / OUTPUT DATA STRUCTURES
// Same signal names from VCU code
// These are not physical pins
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

// ARDUINO TEST-BENCH SIGNAL PINS
// TODO: Finalize these
// Simulated VCU inputs
const int PIN_DRIVER_ACTION = 0;
const int PIN_TS_ACTIVE =  0;

const int PIN_APPS1_SIM = 0;
const int PIN_APPS2_SIM = 0;
const int PIN_BSE_SIM = 0;
// VCU outputs monitored by the test bench
const int PIN_BRAKE_LIGHT_MONITOR = 0;
const int PIN_BUZZER_MONITOR = 0;

// CAN DRIVER OBJECT
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
