/* This code is taken from the CANLogger repo (https://github.com/UCR-FSAE/CANLogger).
* Thanks Justin!
*
* Author: Emad Saadat and Emily Tan (and Justin Im).
*/

#include <SPI.h>
#include "mcp2515_can.h"

const int SPI_CS_PIN = 9;

mcp2515_can CAN(SPI_CS_PIN);

void setup() {
    // Intialize serial port
    SERIAL_PORT_MONITOR.begin(500000);
    while (!SERIAL_PORT_MONITOR) {  }

    // Initialize CAN interface
    while (CAN_OK != CAN.begin(CAN_500KBPS)) {
        SERIAL_PORT_MONITOR.println("CAN init fail, retrying...");
        delay(100);
    }
    SERIAL_PORT_MONITOR.println("CAN init ok!");

    // Setup done
    SERIAL_PORT_MONITOR.println("Starting...");
}

void loop() {
    
}
