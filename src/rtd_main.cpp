/*
* This code is taken from the CANLogger repo (https://github.com/UCR-FSAE/CANLogger).
* Thanks Justin!
*
* Author: Emad Saadat and Emily Tan
*/

#include <Arduino.h>
#include "can.hpp"
#include "util.hpp"

void setup() {
    setupSerialBlocking();
    setupPins();
    //setupDAC();
    setupCANBlocking();
    runRTDTest();
}

void loop() {
    return;
}
