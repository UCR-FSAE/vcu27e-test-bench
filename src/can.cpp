#include "can.hpp"

mcp2515_can CAN(CAN_CS_PIN);

void setupCANBlocking() {
    while (CAN.begin(CAN_500KBPS) != CAN_OK) { delay(100); }
}

bool getMsg(CANMessage &msg) {
    if (CAN.checkReceive() != CAN_MSGAVAIL) return false;
    
    unsigned long id = 0;
    byte len = 0;
    byte buf[CAN_DATA_MAX];
    
    if (CAN.readMsgBufID(&id, &len, buf) != CAN_OK) return false;
    
    msg.valid = true;
    msg.id = (uint16_t) id;
    msg.length = len;
    for (byte i = 0; i < CAN_DATA_MAX; i++) {
        msg.data[i] = (i < len) ? buf[i] : 0;
    }
    
    return true;
}

void waitForMsg(CANMessage &msg) {
    while (!getMsg(msg)) { }
}

void printMsg(const CANMessage &msg) {
    // Must be only used when Serial is initialized
    if (!Serial) return;
    
    Serial.print(msg.id, HEX);
    Serial.print(" = ");
    for (byte i = 0; i < CAN_DATA_MAX; i++) {
        Serial.print(" ");
        Serial.print(msg.data[i], HEX);
    }
    Serial.println("");
}

MsgMatch checkMsg(const CANMessage &msg, const CANMessage &expected) {
    if (msg.id != expected.id) return MsgMatch::WRONG_ID;
    if (msg.length != expected.length) return MsgMatch::WRONG_LENGTH;
    
    for (byte i = 0; i < expected.length; i++) {
        if (msg.data[i] != expected.data[i]) return MsgMatch::WRONG_DATA;
    }
    
    return MsgMatch::MATCHED;
}

void clearMsg(CANMessage &msg) {
    msg.valid = false;
    msg.id = 0;
    msg.length = 0;
    for (byte i = 0; i < msg.length; i++) msg.data[i] = 0;
}

void sendStatusMsg() {
    CAN.trySendMsgBuf(CAN_ID_INVERTER_TO_VCU, 0x00, 0x00, CAN_DATA_MAX, STATUS_DATA);
}
