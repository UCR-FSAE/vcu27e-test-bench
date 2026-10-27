#include <Adafruit_MCP4728.h>
#include <Wire.h>

#include "can.hpp"
#include "util.hpp"

static Adafruit_MCP4728 dac;

static uint16_t voltsToDAC(float volts) {
    // TODO: convert volts to DAC
}

void setupPins() {
    // Set Input Pins
    pinMode(PIN_BRAKE_LIGHT_MONITOR, INPUT);
    pinMode(PIN_BUZZER_MONITOR, INPUT);
    
    // Set Output Pins
    // pinMode(PIN_APPS1_SIM, OUTPUT);
    // pinMode(PIN_APPS2_SIM, OUTPUT);
    // pinMode(PIN_BSE_SIM, OUTPUT);
    pinMode(PIN_TS_ACTIVE_SIM, OUTPUT);
    pinMode(PIN_DRIVER_ACTION_SIM, OUTPUT);
    pinMode(PIN_VCU_RESET, OUTPUT);

    // Set Initial Values
    // digitalWrite(PIN_APPS1_SIM, LOW);
    // digitalWrite(PIN_APPS2_SIM, LOW);
    // digitalWrite(PIN_BSE_SIM, LOW);
    digitalWrite(PIN_TS_ACTIVE_SIM, LOW);
    digitalWrite(PIN_DRIVER_ACTION_SIM, LOW);
    digitalWrite(PIN_VCU_RESET, LOW);
}

bool setupDAC() {
    Wire.begin();
    if (!dac.begin()) {
        Serial.println("DAC not found");
        return false;
    }
    // set idle pedals
    writeDAC(APPS1_MIN_V, APPS2_MIN_V, BSE_RELEASED_V);
    return;
}

bool writeDAC(float apps1_v, float apps2_v, float bse_v) {
    const uint16_t va = voltsToDAC(apps1_v);
    const uint16_t vb = voltsToDAC(apps2_v);
    const uint16_t vc = voltsToDAC(bse_v);
    const uint16_t vd = 0;

    // TODO: write dac values (Emad)
    return;
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

void sendStatusMsg(bool isEnabled, bool isLocked) {
    byte STATUS_DATA[CAN_DATA_MAX] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    STATUS_DATA[6] = (isEnabled ? 0x01 : 0x00) | (isLocked ? 0x80 : 0x00);
    
    CAN.trySendMsgBuf(CAN_ID_INVERTER_TO_VCU, 0x00, 0x00, CAN_DATA_MAX, STATUS_DATA);
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

        if (waitState == WAIT_FOR_DISABLE) { sendStatusMsg(false, true); }
        else if (waitState == WAIT_FOR_ENABLE) { sendStatusMsg(false, false); }
        
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
        delay(30);
    }
    
    sendStatusMsg(true, false);
    Serial.println("Inverter Initialization Successful.");
}

static bool buzzerIsOn(int raw) {
    // check that buzzer is on (600 * 5)/1023 = 2.93V
    return raw > 600;
}

void runRTDTest() {
    enum Phase { WAIT_HIGH, WAIT_LOW };
    Phase phase = WAIT_HIGH;

    unsigned long highStart = 0;
    unsigned long highMs = 0;

    bool buzzDone = false;
    bool buzzOk = false;

    Serial.println("RTD test start");

    // TODO: add DAC control


    Serial.println("Sent RTD sequence");

    while (!buzzDone) {
        bool on = buzzerIsOn(analogRead(PIN_BUZZER_MONITOR));

        switch (phase) {
            case WAIT_HIGH:
                if (on) {
                    highStart = millis();
                    phase = WAIT_LOW;
                    Serial.println("RTD buzzer HIGH");
                }
                break;
            case WAIT_LOW:
                if (!on) {
                    highMs = millis() - highStart;
                    buzzOk = highMs >= BUZZER_TIME_MIN_MS &&
                             highMs <= BUZZER_TIME_MAX_MS;
                    buzzDone = true;

                    Serial.println("RTD buzzer LOW");
                    Serial.print("ms=");
                    Serial.print(highMs);
                    Serial.println(buzzOk ? " in range" : " out of range");
                }
                break;
        }
    }

    waitForInverterInit();

    if (buzzOk) {
        Serial.println("RTD Successful.");
    } else {
        Serial.println("RTD failed.");
    }
}
