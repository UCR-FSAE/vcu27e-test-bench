#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MCP4728.h>
#include "constants.hpp"
#include "can.hpp"

extern Adafruit_MCP4728 dac;

void setupPins();

void setupDAC();
void writeDAC(float apps1_v, float apps2_v, float bse_v);
uint16_t voltsToDAC(float volts);
void setIdlePedals();

void setupSerialBlocking();
void endSerialBlocking();

void sendStatusMsg(bool isEnabled, bool isLocked);
void waitForInverterInit();

void beRTD();
void runRTDTest();

void runBrakeLightTest();
