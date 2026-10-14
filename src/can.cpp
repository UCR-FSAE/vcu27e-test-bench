#include "can.hpp"

mcp2515_can CAN(CAN_CS_PIN);

void setupCANBlocking() {
    while (CAN.begin(CAN_500KBPS) != CAN_OK) { delay(100); }
}
