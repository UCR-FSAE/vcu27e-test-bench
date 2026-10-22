#pragma once

/* All Constants Go Here */

// Serial

#define SERIAL_BAUDRATE 500000ULL

// CAN Constants

// CAN Shield Configuration
#define CAN_CS_PIN 9
#define CAN_BAUDRATE 500000ULL

// CAN Messages
#define CAN_DATA_MAX 8

// CAN IDS

// Inverter
#define CAN_ID_VCU_TO_INVERTER 0x0C0
#define CAN_ID_INVERTER_CLEAR 0x0C1
#define CAN_ID_INVERTER_TO_VCU 0x0AA

// Inverter Init Data
#define DISABLE_CMD_DATA { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
#define ENABLE_CMD_DATA { 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00 }

// Inverter Commands
#define INVERTER_DIRECTION_FORWARD 0x00
#define INVERTER_DIRECTION_REVERSE 0x01
#define INVERTER_ENABLE 0x01
#define INVERTER_DISABLE 0x00
// InverterCheck() bit 0 of byte 6
#define INVERTER_STATUS_ENABLED 0x01 
// bit 7 of byte 6, 0 = inverter is NOT locked, 1 = inverter is locked
#define INVERTER_STATUS_LOCKOUT 0x80

// Timings
#define INVERTER_INIT_GAP_MS 500 
#define BUZZER_TIME_MS 1500 
#define BUZZER_TIME_MIN_MS 1000 
#define BUZZER_TIME_MAX_MS 3000 
#define CAN_FRAME_PERIOD_MS 30 
#define APPS_IMPLAUSIBILITY_TIME_MS 100 

// Signal Values
#define APPS1_MIN_V 1.4f 
#define APPS1_MAX_V 3.2f 
#define APPS2_MIN_V 0.4f 
#define APPS2_MAX_V 1.4f 
#define BSE_MIN_V 0.3f 
#define BSE_MAX_V 1.3f 
#define BRAKES_ACTIVATION_THRESHOLD_V 0.5f 
#define APPS_IMPLAUSIBILITY_THRESHOLD_PERCENT 20.0f 
#define APPS_RANGE_LOW_PERCENT 0.0f 
#define APPS_RANGE_HIGH_PERCENT 110.0f 
#define DEADZONE_PERCENT 15.0f 
#define MAX_TORQUE_COMMAND 4000.0f 

// Pedal Test Values
const uint8_t PEDAL_TEST_VALUES[6] = { 0, 10, 25, 50, 75, 100 };

// Pins
#define PIN_APPS1_SIM 3
#define PIN_APPS2_SIM 5
#define PIN_BSE_SIM 6
#define PIN_DRIVER_ACTION_SIM 7
#define PIN_TS_ACTIVE_SIM 8
#define PIN_VCU_RESET A2

#define PIN_BRAKE_LIGHT_MONITOR A0
#define PIN_BUZZER_MONITOR A1
