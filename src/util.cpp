#include "can.hpp"
#include "util.hpp"

void setupPins() {
    // Set Input Pins
    pinMode(PIN_BRAKE_LIGHT_MONITOR, INPUT);
    pinMode(PIN_BUZZER_MONITOR, INPUT);
    
    // Set Output Pins
    pinMode(PIN_APPS1_SIM, OUTPUT);
    pinMode(PIN_APPS2_SIM, OUTPUT);
    pinMode(PIN_BSE_SIM, OUTPUT);
    pinMode(PIN_TS_ACTIVE_SIM, OUTPUT);
    pinMode(PIN_DRIVER_ACTION_SIM, OUTPUT);
    pinMode(PIN_VCU_RESET, OUTPUT);
    
    // Set Initial Values
    digitalWrite(PIN_APPS1_SIM, LOW);
    digitalWrite(PIN_APPS2_SIM, LOW);
    digitalWrite(PIN_BSE_SIM, LOW);
    digitalWrite(PIN_TS_ACTIVE_SIM, LOW);
    digitalWrite(PIN_DRIVER_ACTION_SIM, LOW);
    digitalWrite(PIN_VCU_RESET, LOW);
}

void resetVCU() {
    digitalWrite(PIN_VCU_RESET, LOW);
    delay(100);
    digitalWrite(PIN_VCU_RESET, HIGH);
    delay(100);
    digitalWrite(PIN_VCU_RESET, LOW);
}

void setupSerialBlocking() {
    Serial.begin(SERIAL_BAUDRATE);
    while (!Serial) {  }
}

void endSerialBlocking() {
    Serial.end();
    while (Serial) { }
}

void waitForInverterInit() {
    enum WaitState { WAIT_FOR_DISABLE, WAIT_FOR_ENABLE, SUCCESS } waitState;
    waitState = WAIT_FOR_DISABLE;
    CANMessage lastMsg;
    
    // Must be only used when Serial is initialized
    if (!Serial) return;
    
    while (waitState != SUCCESS) {
        Serial.println("Waiting for inverter initialization sequence...");
        
        waitForMsg(lastMsg);
        printMsg(lastMsg);
        
        switch (waitState) {
            case WAIT_FOR_DISABLE:
                switch (checkMsg(lastMsg, EXPECTED_DISABLE_CMD)) {
                    case MsgMatch::WRONG_ID:
                        Serial.println("Missed Disable CMD");
                        break;
                    case MsgMatch::WRONG_LENGTH:
                        Serial.println("Wrong Disable Length");
                        break;
                    case MsgMatch::WRONG_DATA:
                        Serial.println("Wrong Disable Data");
                        break;
                    case MsgMatch::MATCHED:
                        Serial.println("Got Disable CMD");
                        waitState = WAIT_FOR_ENABLE;
                        break;
                }
                break;
            case WAIT_FOR_ENABLE:
                switch (checkMsg(lastMsg, EXPECTED_ENABLE_CMD)) {
                    case MsgMatch::WRONG_ID:
                        Serial.println("Missed Enable CMD");
                        break;
                    case MsgMatch::WRONG_LENGTH:
                        Serial.println("Wrong Enable Length");
                        break;
                    case MsgMatch::WRONG_DATA:
                        Serial.println("Wrong Enable Data");
                        break;
                    case MsgMatch::MATCHED:
                        Serial.println("Got Enable CMD");
                        waitState = SUCCESS;
                        break;
                }
                break;
            case SUCCESS:
                break;
        }
        
        clearMsg(lastMsg);
        delay(100);
    }
    
    Serial.println("Inverter Initialization Successful.");
}
