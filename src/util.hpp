#pragma once

#include <Arduino.h>
#include "constants.hpp"

void setupPins();
bool setupDAC();
bool writeDAC(float apps1_v, float apps2_v, float bse_v);
void resetVCU();
void setupSerialBlocking();
void endSerialBlocking();
void sendStatusMsg(bool isEnabled, bool isLocked);
void waitForInverterInit();
void runRTDTest();
