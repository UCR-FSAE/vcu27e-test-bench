#pragma once

#include <Arduino.h>
#include "constants.hpp"

byte STATUS_DATA[CAN_DATA_MAX] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

void setupPins();
void resetVCU();
void setupSerialBlocking();
void endSerialBlocking();
void sendStatusMsg(bool isEnabled, bool isLocked);
void waitForInverterInit();
