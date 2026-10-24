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
    setupCANBlocking();
    resetVCU();
    //waitForInverterInit(); // commented to test rtd test, uncomment before merging
    runRTDTest();
}

void loop() {
    static unsigned long lastRunMS = 0;
    static CANMessage lastMsg;
    
    if (millis() - lastRunMS < 10) { return; }
    lastRunMS = millis();
    
    if (getMsg(lastMsg)) printMsg(lastMsg);
    sendStatusMsg(true, false);
}
