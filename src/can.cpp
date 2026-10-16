#include "can.hpp"

mcp2515_can CAN(CAN_CS_PIN);

void setupCANBlocking() {
    while (CAN.begin(CAN_500KBPS) != CAN_OK) { delay(100); }
}

bool getMsg(CANMessage &msg) {
    if (CAN.checkReceive() != CAN_MSGAVAIL) return false;
    
    unsigned long id = 0;
    unsigned char len = 0;
    unsigned char buf[8];
    
    if (CAN.readMsgBufID(&id, &len, buf) != CAN_OK) return false;
    
    msg.valid = true;
    msg.id = id;
    msg.length = len;
    for (unsigned char i = 0; i < 8; i++) {
        msg.data[i] = (i < len) ? buf[i] : 0;
    }
    
    return true;
}

void waitForMsg(CANMessage &msg) {
    while (!getMsg(msg)) {  }
}

void clearMsg(CANMessage &msg) {
    msg.valid = false;
    msg.id = 0;
    msg.length = 0;
    for (unsigned char i = 0; i < msg.length; i++) msg.data[i] = 0;
}
