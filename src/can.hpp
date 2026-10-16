#pragma once

#include <Arduino.h>
#include "mcp2515_can.h"
#include "constants.hpp"

extern mcp2515_can CAN;

struct CANMessage {
    bool valid;
    unsigned long id;
    byte length;
    byte data[8];
};

void setupCANBlocking();
bool getMsg(CANMessage &msg);
void waitForMsg(CANMessage &msg);
void clearMsg(CANMessage &msg);
