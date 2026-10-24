#pragma once

#include <Arduino.h>
#include "constants.hpp"

void setupPins();
void resetVCU();
void setupSerialBlocking();
void endSerialBlocking();
void sendStatusMsg(bool isEnabled, bool isLocked);
void waitForInverterInit();
void runRTDTest();
