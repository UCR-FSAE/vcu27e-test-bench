#pragma once

#include <Arduino.h>
#include "mcp2515_can.h"
#include "constants.hpp"

extern mcp2515_can CAN;

struct CANMessage {
    bool valid;
    uint16_t id;
    byte length;
    byte data[CAN_DATA_MAX];
};

enum class MsgMatch { MATCHED, WRONG_ID, WRONG_LENGTH, WRONG_DATA };

const CANMessage EXPECTED_DISABLE_CMD = {
    false,
    CAN_ID_VCU_TO_INVERTER,
    CAN_DATA_MAX,
    DISABLE_CMD_DATA
};

const CANMessage EXPECTED_ENABLE_CMD = {
    false,
    CAN_ID_VCU_TO_INVERTER,
    CAN_DATA_MAX,
    ENABLE_CMD_DATA
};

void setupCANBlocking();
bool getMsg(CANMessage &msg);
void waitForMsg(CANMessage &msg);
void printMsg(const CANMessage &msg);
MsgMatch checkMsg(const CANMessage &msg, const CANMessage &expected);
void clearMsg(CANMessage &msg);
void sendStatusMsg();
