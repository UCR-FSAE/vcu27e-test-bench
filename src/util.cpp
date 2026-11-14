#include "util.hpp"

Adafruit_MCP4728 dac;

void setupPins() {
    // Set Input Pins
    pinMode(PIN_BRAKE_LIGHT_MONITOR, INPUT);
    pinMode(PIN_BUZZER_MONITOR, INPUT);
    
    // Set Output Pins
    pinMode(PIN_TS_ACTIVE_SIM, OUTPUT);
    pinMode(PIN_DRIVER_ACTION_SIM, OUTPUT);
    pinMode(PIN_VCU_RESET, OUTPUT);

    // Set Initial Values
    digitalWrite(PIN_TS_ACTIVE_SIM, LOW);
    digitalWrite(PIN_DRIVER_ACTION_SIM, HIGH);
    digitalWrite(PIN_VCU_RESET, LOW);
}

void setupDAC() {
    Wire.begin();
    if (!dac.begin()) {
        Serial.println("DAC not found");
        return;
    }
    
    setIdlePedals();
}

void writeDAC(float apps1_v, float apps2_v, float bse_v) {
    uint16_t va = voltsToDAC(apps1_v);
    uint16_t vb = voltsToDAC(apps2_v);
    uint16_t vc = voltsToDAC(bse_v);
    uint16_t vd = 0;

    dac.setChannelValue(MCP4728_CHANNEL_A, va, MCP4728_VREF_VDD, MCP4728_GAIN_1X);
    dac.setChannelValue(MCP4728_CHANNEL_B, vb, MCP4728_VREF_VDD, MCP4728_GAIN_1X);
    dac.setChannelValue(MCP4728_CHANNEL_C, vc, MCP4728_VREF_VDD, MCP4728_GAIN_1X);
    dac.setChannelValue(MCP4728_CHANNEL_D, vd, MCP4728_VREF_VDD, MCP4728_GAIN_1X);
}

void setIdlePedals() {
    writeDAC(APPS1_MIN_V, APPS2_MIN_V, BSE_MIN_V);
}

uint16_t voltsToDAC(float volts) {
    return (volts / 3.3f) * 4096;
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

void beRTD() {
    digitalWrite(PIN_TS_ACTIVE_SIM, HIGH);
    writeDAC(APPS1_MIN_V, APPS2_MIN_V, BRAKES_ACTIVATION_THRESHOLD_V + .5f);
    digitalWrite(PIN_DRIVER_ACTION_SIM, HIGH);
}

// TODO: Implement case when tractive system is disabled in middle of sequence.
void runRTDTest() {
    enum TestState { NOTHING, WAIT_BUZZ_HIGH, WAIT_BUZZ_LOW, DONE };
    TestState state = NOTHING;

    unsigned long highStart = 0;
    unsigned long highMs = 0;
    bool buzzerOn = false;
    bool buzzOk = false;

    Serial.println("RTD test start");

    while (state != DONE) {
        buzzerOn = digitalRead(PIN_BUZZER_MONITOR);
        
        switch (state) {
            case NOTHING:
                beRTD();
                state = WAIT_BUZZ_HIGH;
                break;
            case WAIT_BUZZ_HIGH:
                if (buzzerOn) {
                    highStart = millis();
                    state = WAIT_BUZZ_LOW;
                    Serial.println("RTD buzzer HIGH");
                }
                break;
            case WAIT_BUZZ_LOW:
                if (!buzzerOn) {
                    highMs = millis() - highStart;
                    buzzOk = highMs >= BUZZER_TIME_MIN_MS &&
                             highMs <= BUZZER_TIME_MAX_MS;

                    Serial.println("RTD buzzer LOW");
                    Serial.print("ms=");
                    Serial.print(highMs);
                    Serial.println(buzzOk ? " in range" : " out of range");
                    state = DONE;
                }
                break;
            case DONE:
                break;
        }
    }

    if (buzzOk) {
        Serial.println("RTD Successful.");
    } else {
        Serial.println("RTD failed.");
    }
}

void runBrakeLightTest() {
    writeDAC(APPS1_MIN_V, APPS2_MIN_V, BRAKES_ACTIVATION_THRESHOLD_V - 0.1f);
    delay(100);
    
    Serial.println("Brake not pressed, should not be on");
    if (!digitalRead(PIN_BRAKE_LIGHT_MONITOR)) {
        Serial.println("True");
    } else {
        Serial.println("False");
    }
    
    writeDAC(APPS1_MIN_V, APPS2_MIN_V, BRAKES_ACTIVATION_THRESHOLD_V + 2.0f);
    delay(100);
    
    Serial.println("Brake pressed, should be on");
    if (digitalRead(PIN_BRAKE_LIGHT_MONITOR)) {
        Serial.println("True");
    } else {
        Serial.println("False");
    }
    
    writeDAC(APPS1_MIN_V, APPS2_MIN_V, BRAKES_ACTIVATION_THRESHOLD_V - 0.1f);
    delay(100);
    
    Serial.println("Brake not pressed, should not be on");
    if (!digitalRead(PIN_BRAKE_LIGHT_MONITOR)) {
        Serial.println("True");
    } else {
        Serial.println("False");
    }
}
