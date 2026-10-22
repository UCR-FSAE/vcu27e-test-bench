/*
* CAN SNIFFER for Arduino Uno R3 + Seeed Studio CAN-BUS Shield V2 (MCP2515)
*
* Passively listens to the PM100 / RMS-Cascadia inverter bus and prints
* decoded frames on Serial. Decoding follows the "CAN Protocol V6.3" document.
*
* Original test bench taken from the CANLogger repo (https://github.com/UCR-FSAE/CANLogger).
* Thanks Justin!
*
* Original author: Emad Saadat and Emily Tan
*
* Hardware: Seeed CAN-BUS Shield V2 -> CS = D9, INT = D2 (INT unused, we poll)
* Library : Seeed_Arduino_CAN (mcp2515_can)
* Serial  : SERIAL_BAUDRATE from constants.hpp (500000)
*
* Serial commands (send a single character):
*   r  toggle RAW CSV mode (every frame, no throttling: ms,id,len,b0..b7)
*   f  toggle "print every frame" (decoded mode, no per-ID throttling)
*   h  help
*
* Lines starting with '#' are sniffer messages (not CAN data).
*/

#include <Arduino.h>
#include "constants.hpp"
#include "can.hpp"
#include "util.hpp"

// =====================================================================
//  OLD TEST BENCH CODE (disabled).  "#if 0" is used instead of /* */
//  because the old code already contains block comments, which don't nest.
// =====================================================================
#if 0

// Sim Output Variables
int apps1High = 0;
int apps2High = 0;
int bseHigh = 0;
int driverHigh = 0; // LOW(0) means pressed
int tsHigh = 0;

// Loop Variables
unsigned long lastRunMS = 0;
CANMessage lastMsg;
bool init_success = false;
bool inverter_disabled_cmd = false;
bool inverter_enabled_cmd = false;

enum inverter_state { ENABLED, DISABLED, STARTUP };
inverter_state inv_state;

const byte statusBuf[8] = {0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00};

void setup() {
    inv_state = STARTUP;
    setupSerialBlocking();
    setupPins();
    setupCANBlocking();
    resetVCU();
    
    // Init Check
    
    while (!init_success) {
        if (inv_state == STARTUP) {
            Serial.println("Waiting for inverter initialization sequence...");
        }
        
        waitForMsg(lastMsg);
        
        Serial.print(lastMsg.id, HEX);
        Serial.print(" = ");
        for (unsigned char i = 0; i < 8; i++) {
            Serial.print(" ");
            Serial.print(lastMsg.data[i], HEX);
        }
        Serial.println("");
        
        if (lastMsg.valid && inv_state == STARTUP) {
            if (lastMsg.id != CAN_ID_VCU_TO_INVERTER || lastMsg.length != 8) {
                Serial.println("Missed Disable CMD");
            } else {
                for (unsigned char i = 0; i < 8; i++) {
                    if (lastMsg.data[i] != 0x00) {
                        Serial.println("Missed Disable CMD");
                    } else {
                        inverter_disabled_cmd = true;
                    }
                }
                if (inverter_disabled_cmd) { 
                    inv_state = DISABLED; 
                    Serial.println("Inverter disabled");
                }
            }
        } else if (inv_state == DISABLED) {
            Serial.println("Sending inverter status in 1.5 s");
            delay(1500);
            CAN.trySendMsgBuf(CAN_ID_INVERTER_TO_VCU, 0x00, 0x00, 0x08, statusBuf); 
            delay(15);   
            if (lastMsg.id != CAN_ID_VCU_TO_INVERTER || lastMsg.length != 8) {
                Serial.println("Missed Enable CMD");
            } else {
                for (unsigned char i = 0; i < 8; i++) {
                    if (i != 4 && i != 5) {
                        if (lastMsg.data[i] != 0x00) {
                            Serial.println("Incorrect Enable CMD");
                        }
                    }
                }

                if (lastMsg.data[4] != 0x00 || lastMsg.data[5] != 0x01) {
                    Serial.println("Incorrect Enable CMD");
                } else {
                    inv_state = ENABLED;
                    init_success = true;
                }
            }
        }
        clearMsg(lastMsg);
        delay(10);
    }
    
    Serial.println("Inverter Initialization Successful.");
}

void loop() {
    // -------------- CAN frame reading --------------
    // Read CAN frame 0x0C0 (CAN_ID_VCU_TO_INVERTER): stays 0 if not connected    
    if (getMsg(lastMsg)) {
        if (lastMsg.id != CAN_ID_VCU_TO_INVERTER) clearMsg(lastMsg);
    }
    
    // -------------- Print output every 0.25 s--------------
    if (millis() - lastRunMS < 250) { return; }
    lastRunMS = millis();
    
    /* Expected output with vcu not connected:
    time: 250 ms
        SENT driver=HIGH ts=LOW apps1=LOW apps2=LOW bse=LOW | CAN tx none
        RECV brake=LOW buzzer=LOW | CAN rx none
    with a frame: CAN rx 0x0C0 = A0 0F 00 00 00 01 00 00 torque=4000
    */
    // analogRead is 0 at 0 V and 1023 at 5 V.
    float brakeV = analogRead(PIN_BRAKE_LIGHT_MONITOR) * (5.0f / 1023.0f);
    float buzzerV = analogRead(PIN_BUZZER_MONITOR) * (5.0f / 1023.0f);

    Serial.print("time: ");
    Serial.print(lastRunMS);
    Serial.println(" ms");

    Serial.print("  SENT driver=");
    Serial.print(driverHigh ? "HIGH" : "LOW");
    Serial.print(" ts=");
    Serial.print(tsHigh ? "HIGH" : "LOW");
    Serial.print(" apps1=");
    Serial.print(apps1High ? "HIGH" : "LOW");
    Serial.print(" apps2=");
    Serial.print(apps2High ? "HIGH" : "LOW");
    Serial.print(" bse=");
    Serial.print(bseHigh ? "HIGH" : "LOW");
    Serial.println(" | CAN tx none");

    Serial.print("  RECV brake=");
    Serial.print(brakeV, 2);
    Serial.print("V buzzer=");
    Serial.print(buzzerV, 2);
    Serial.print("V");
    Serial.print(" | CAN rx ");
    if (lastMsg.valid) {
        Serial.print(lastMsg.id, HEX);
        Serial.print(" = ");
        
        for (unsigned char i = 0; i < 8; i++) {
            Serial.print(" ");
            if (lastMsg.data[i] < 16) {
                Serial.print(lastMsg.data[i], DEC);
            } else {
                Serial.print("0x");
                Serial.print(lastMsg.data[i], HEX);
            }
        }
        
        // bytes 0 and 1 are little-endian torque, byte 5 is enable inverter
        unsigned int torque =
            lastMsg.data[0] | ((unsigned int)lastMsg.data[1] << 8);
        Serial.print(" torque=");
        Serial.print(torque);
        clearMsg(lastMsg);
    } else {
        Serial.print("none");
    }
    Serial.println();

    if (millis() - lastRunMS >= 100) { 
        const byte statusData[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00};
        CAN.trySendMsgBuf(CAN_ID_INVERTER_TO_VCU, 0x00, 0x00, 0x08, statusData);
    }
    
    lastRunMS = millis();
}

#endif  // old test bench


// =====================================================================
//  SNIFFER CONFIG
// =====================================================================

// Must match the inverter's CAN_Bit_Rate_EEPROM. PM100 factory default is
// 250 kbps (CAN_250KBPS); your VCU code uses 500 kbps.
#define SNIFFER_CAN_SPEED   CAN_500KBPS

// 1 = put the MCP2515 in listen-only mode (never ACKs, never drives the bus).
// Safer if you're unsure about the baud rate. Needs a library version that
// exposes CAN.setMode() / MODE_LISTENONLY; leave 0 if it doesn't compile.
#define SNIFFER_LISTEN_ONLY 0

#define DEFAULT_INTERVAL_MS 100UL   // min time between prints of the same ID
#define SILENCE_WARN_MS     2000UL  // warn if no frame for this long
#define STATS_PERIOD_MS     5000UL  // frames/sec summary
#define MAX_FRAMES_PER_PASS 16      // frames drained from the MCP2515 per loop

// =====================================================================
//  Per-ID slots (keeps RAM small on the Uno: 22 x 16 bytes)
//    0..17  -> 0x0A0..0x0B1 (inverter broadcasts)
//    18..20 -> 0x0C0..0x0C2 (command / parameter messages)
//    21     -> anything else
// =====================================================================
#define NUM_SLOTS  22
#define SLOT_OTHER 21

struct Slot {
    uint8_t data[8];
    uint8_t len;            // 0 = never received
    bool fresh;             // newer than the last print
    bool urgent;            // print ASAP (important change)
    unsigned long lastPrint;
};

Slot slots[NUM_SLOTS];
unsigned long otherId = 0;
uint8_t rrNext = 0;

bool rawMode = false;
unsigned long intervalMs = DEFAULT_INTERVAL_MS;

unsigned long frameCount = 0;
unsigned long lastStatsMs = 0;
unsigned long lastRxMs = 0;
bool silentWarned = false;

// ---------------------------------------------------------------------
//  Small helpers (no float, no printf: cheap on AVR)
// ---------------------------------------------------------------------
static int16_t rdS16(const uint8_t *d, uint8_t i) {
    return (int16_t)((uint16_t)d[i] | ((uint16_t)d[i + 1] << 8));
}
static uint16_t rdU16(const uint8_t *d, uint8_t i) {
    return (uint16_t)d[i] | ((uint16_t)d[i + 1] << 8);
}
static uint32_t rdU32(const uint8_t *d, uint8_t i) {
    return (uint32_t)d[i] | ((uint32_t)d[i + 1] << 8) |
           ((uint32_t)d[i + 2] << 16) | ((uint32_t)d[i + 3] << 24);
}

static void printHex2(uint8_t b) {
    if (b < 0x10) Serial.print('0');
    Serial.print(b, HEX);
}

static void printId(unsigned long id) {
    if (id <= 0x7FF) {                 // standard frame: pad to 3 digits
        if (id < 0x100) Serial.print('0');
        if (id < 0x10)  Serial.print('0');
    }
    Serial.print(id, HEX);
}

// value is "x10" scaled (temperature, voltage, current, torque, angle, freq)
static void printTenths(int32_t v) {
    if (v < 0) { Serial.print('-'); v = -v; }
    Serial.print((long)(v / 10));
    Serial.print('.');
    Serial.print((int)(v % 10));
}

static void fieldT(const __FlashStringHelper *name, int16_t v10) {
    Serial.print(' '); Serial.print(name); Serial.print('=');
    printTenths(v10);
}

static void fieldI(const __FlashStringHelper *name, long v) {
    Serial.print(' '); Serial.print(name); Serial.print('=');
    Serial.print(v);
}

static void printBytes(const uint8_t *d, uint8_t len) {
    Serial.print(F(" |"));
    for (uint8_t i = 0; i < len && i < 8; i++) {
        Serial.print(' ');
        printHex2(d[i]);
    }
}

// ---------------------------------------------------------------------
//  Name tables (flash strings via switch, no RAM cost)
// ---------------------------------------------------------------------
static void printVsmState(uint8_t s) {
    switch (s) {
        case 0:  Serial.print(F("START")); break;
        case 1:  Serial.print(F("PRECHG_INIT")); break;
        case 2:  Serial.print(F("PRECHG_ACTIVE")); break;
        case 3:  Serial.print(F("PRECHG_DONE")); break;
        case 4:  Serial.print(F("WAIT")); break;
        case 5:  Serial.print(F("READY")); break;
        case 6:  Serial.print(F("RUNNING")); break;
        case 7:  Serial.print(F("FAULT")); break;
        case 14: Serial.print(F("SHUTDOWN")); break;
        case 15: Serial.print(F("RECYCLE_PWR")); break;
        default: Serial.print('?'); break;
    }
}

static void printInvState(uint8_t s) {
    switch (s) {
        case 0:  Serial.print(F("POWER_ON")); break;
        case 1:  Serial.print(F("STOP")); break;
        case 2:  Serial.print(F("OPEN_LOOP")); break;
        case 3:  Serial.print(F("CLOSED_LOOP")); break;
        case 4:  Serial.print(F("WAIT")); break;
        case 8:  Serial.print(F("IDLE_RUN")); break;
        case 9:  Serial.print(F("IDLE_STOP")); break;
        default: Serial.print(F("internal")); break;
    }
}

static const __FlashStringHelper *postFaultName(uint8_t b) {
    switch (b) {
        case 0:  return F("GATE_DESAT");
        case 1:  return F("HW_OVERCURRENT");
        case 2:  return F("ACCEL_SHORT");
        case 3:  return F("ACCEL_OPEN");
        case 4:  return F("CURR_SENS_LO");
        case 5:  return F("CURR_SENS_HI");
        case 6:  return F("MODULE_TEMP_LO");
        case 7:  return F("MODULE_TEMP_HI");
        case 8:  return F("CTRL_PCB_TEMP_LO");
        case 9:  return F("CTRL_PCB_TEMP_HI");
        case 10: return F("GDB_TEMP_LO");
        case 11: return F("GDB_TEMP_HI");
        case 12: return F("5V_LO");
        case 13: return F("5V_HI");
        case 14: return F("12V_LO");
        case 15: return F("12V_HI");
        case 16: return F("2V5_LO");
        case 17: return F("2V5_HI");
        case 18: return F("1V5_LO");
        case 19: return F("1V5_HI");
        case 20: return F("DCBUS_HI");
        case 21: return F("DCBUS_LO");
        case 22: return F("PRECHG_TIMEOUT");
        case 23: return F("PRECHG_VOLT_FAIL");
        case 24: return F("EEPROM_CHECKSUM");
        case 25: return F("EEPROM_RANGE");
        case 26: return F("EEPROM_UPDATE_REQ");
        case 27: return F("HW_DCBUS_OV_INIT");
        case 28: return F("GATE_DRV_INIT");
        case 30: return F("BRAKE_SHORT");
        case 31: return F("BRAKE_OPEN");
        default: return nullptr;
    }
}

static const __FlashStringHelper *runFaultName(uint8_t b) {
    switch (b) {
        case 0:  return F("OVERSPEED");
        case 1:  return F("OVERCURRENT");
        case 2:  return F("OVERVOLTAGE");
        case 3:  return F("INV_OVERTEMP");
        case 4:  return F("ACCEL_SHORT");
        case 5:  return F("ACCEL_OPEN");
        case 6:  return F("DIRECTION");
        case 7:  return F("INV_RESP_TIMEOUT");
        case 8:  return F("HW_DESAT");
        case 9:  return F("HW_OVERCURRENT");
        case 10: return F("UNDERVOLTAGE");
        case 11: return F("CAN_CMD_LOST");
        case 12: return F("MOTOR_OVERTEMP");
        case 16: return F("BRAKE_SHORT");
        case 17: return F("BRAKE_OPEN");
        case 18: return F("MODA_OVERTEMP");
        case 19: return F("MODB_OVERTEMP");
        case 20: return F("MODC_OVERTEMP");
        case 21: return F("PCB_OVERTEMP");
        case 22: return F("GDB1_OVERTEMP");
        case 23: return F("GDB2_OVERTEMP");
        case 24: return F("GDB3_OVERTEMP");
        case 25: return F("CURRENT_SENSOR");
        case 26: return F("HW_DCBUS_OV");
        case 28: return F("HW_DCBUS_OV_G5");
        case 30: return F("RESOLVER_NOT_CONN");
        default: return nullptr;
    }
}

static void printFaultNames(uint32_t word, bool isRun) {
    bool first = true;
    for (uint8_t b = 0; b < 32; b++) {
        if (!(word & ((uint32_t)1 << b))) continue;
        if (!first) Serial.print(',');
        first = false;
        const __FlashStringHelper *n = isRun ? runFaultName(b) : postFaultName(b);
        if (n) Serial.print(n);
        else { Serial.print(F("bit")); Serial.print(b); }
    }
}

// ---------------------------------------------------------------------
//  Decoder: one human readable line per frame
// ---------------------------------------------------------------------
static void printLine(unsigned long id, const uint8_t *d, uint8_t len) {
    Serial.print('[');
    Serial.print(millis());
    Serial.print(F("] "));
    printId(id);
    Serial.print(' ');

    if (len != 8) {
        Serial.print(F("LEN="));
        Serial.print(len);
        printBytes(d, len);
        Serial.println();
        return;
    }

    switch (id) {
        case 0x0A0:
            Serial.print(F("TEMP1"));
            fieldT(F("modA_C"), rdS16(d, 0));
            fieldT(F("modB_C"), rdS16(d, 2));
            fieldT(F("modC_C"), rdS16(d, 4));
            fieldT(F("gdb_C"),  rdS16(d, 6));
            break;

        case 0x0A1:
            Serial.print(F("TEMP2"));
            fieldT(F("ctrl_C"), rdS16(d, 0));
            fieldT(F("rtd1_C"), rdS16(d, 2));
            fieldT(F("rtd2_C"), rdS16(d, 4));
            break;

        case 0x0A2:
            Serial.print(F("TEMP3"));
            fieldT(F("coolant_C"), rdS16(d, 0));
            fieldT(F("hotspot_C"), rdS16(d, 2));
            fieldT(F("motor_C"),   rdS16(d, 4));
            fieldT(F("shudder_Nm"), rdS16(d, 6));
            break;

        case 0x0A3:
            Serial.print(F("ANALOG"));   // layout depends on firmware version
            break;

        case 0x0A4:
            Serial.print(F("DIN"));
            fieldI(F("fwd"),   d[0]);
            fieldI(F("rev"),   d[1]);
            fieldI(F("brake"), d[2]);
            fieldI(F("regen"), d[3]);
            fieldI(F("ign"),   d[4]);
            fieldI(F("start"), d[5]);
            break;

        case 0x0A5:
            Serial.print(F("MOTOR"));
            fieldT(F("angle_deg"), rdS16(d, 0));
            fieldI(F("speed_rpm"), rdS16(d, 2));
            fieldT(F("freq_Hz"),   rdS16(d, 4));
            fieldT(F("dRes_deg"),  rdS16(d, 6));
            break;

        case 0x0A6:
            Serial.print(F("CURR"));
            fieldT(F("A_A"),  rdS16(d, 0));
            fieldT(F("B_A"),  rdS16(d, 2));
            fieldT(F("C_A"),  rdS16(d, 4));
            fieldT(F("DC_A"), rdS16(d, 6));
            break;

        case 0x0A7:
            Serial.print(F("VOLT"));
            fieldT(F("DCbus_V"), rdS16(d, 0));
            fieldT(F("Vout_V"),  rdS16(d, 2));
            fieldT(F("Vd_V"),    rdS16(d, 4));
            fieldT(F("Vq_V"),    rdS16(d, 6));
            break;

        case 0x0A8:
            Serial.print(F("FLUX"));
            fieldT(F("Id_A"), rdS16(d, 4));
            fieldT(F("Iq_A"), rdS16(d, 6));
            break;

        case 0x0A9: Serial.print(F("INTV"));   break;

        case 0x0AA:
            Serial.print(F("STATE vsm="));
            Serial.print(d[0]); Serial.print('/'); printVsmState(d[0]);
            Serial.print(F(" inv="));
            Serial.print(d[2]); Serial.print('/'); printInvState(d[2]);
            Serial.print(F(" mode="));
            Serial.print((d[4] & 0x01) ? F("SPEED") : F("TORQUE"));
            fieldI(F("en"),      d[6] & 0x01);
            fieldI(F("lockout"), (d[6] >> 7) & 0x01);
            break;

        case 0x0AB: {
            uint32_t post = rdU32(d, 0);
            uint32_t run  = rdU32(d, 4);
            Serial.print(F("FAULT post=0x"));
            Serial.print(post, HEX);
            Serial.print(F(" run=0x"));
            Serial.print(run, HEX);
            if (!post && !run) {
                Serial.print(F(" none"));
            } else {
                if (post) { Serial.print(F(" POST["));  printFaultNames(post, false); Serial.print(']'); }
                if (run)  { Serial.print(F(" RUN["));   printFaultNames(run, true);   Serial.print(']'); }
            }
            break;
        }

        case 0x0AC: {
            uint32_t cnt = rdU32(d, 4);       // counts of 3 ms
            Serial.print(F("TORQUE"));
            fieldT(F("cmd_Nm"), rdS16(d, 0));
            fieldT(F("fb_Nm"),  rdS16(d, 2));
            fieldI(F("up_s"),   (long)(cnt / 333UL));
            break;
        }

        case 0x0AD: Serial.print(F("MODIDX")); break;
        case 0x0AE: Serial.print(F("FWINFO")); break;
        case 0x0AF: Serial.print(F("DIAG"));   break;
        case 0x0B0: Serial.print(F("HISPD"));  break;
        case 0x0B1: Serial.print(F("TQCAP"));  break;

        case 0x0C0:
            Serial.print(F("CMD"));
            fieldT(F("torque_Nm"), rdS16(d, 0));
            fieldI(F("speed"),     rdS16(d, 2));
            Serial.print(F(" dir="));
            Serial.print(d[4] ? F("FWD") : F("REV"));
            fieldI(F("en"),    d[5] & 0x01);
            fieldI(F("disch"), (d[5] >> 1) & 0x01);
            fieldI(F("spdmode"), (d[5] >> 2) & 0x01);
            fieldT(F("tlim_Nm"), rdS16(d, 6));
            break;

        case 0x0C1:
            Serial.print(F("PARAM_CMD addr="));
            Serial.print(rdU16(d, 0));
            Serial.print(d[2] ? F(" WRITE") : F(" READ"));
            fieldI(F("data"), rdU16(d, 4));
            break;

        case 0x0C2:
            Serial.print(F("PARAM_RSP addr="));
            Serial.print(rdU16(d, 0));
            fieldI(F("ok"),   d[2]);
            fieldI(F("data"), rdU16(d, 4));
            break;

        default:
            Serial.print(F("OTHER"));
            break;
    }

    printBytes(d, len);
    Serial.println();
}

// ---------------------------------------------------------------------
//  RAW CSV: ms,id,len,b0,...,b7
// ---------------------------------------------------------------------
static void printRawCsv(unsigned long id, const CANMessage &m) {
    Serial.print(millis());
    Serial.print(',');
    printId(id);
    Serial.print(',');
    Serial.print(m.length);
    for (uint8_t i = 0; i < m.length && i < 8; i++) {
        Serial.print(',');
        printHex2(m.data[i]);
    }
    Serial.println();
}

// ---------------------------------------------------------------------
//  ID mapping
// ---------------------------------------------------------------------
static uint8_t slotForId(unsigned long id) {
    if (id >= 0x0A0 && id <= 0x0B1) return (uint8_t)(id - 0x0A0);
    if (id >= 0x0C0 && id <= 0x0C2) return (uint8_t)(18 + (id - 0x0C0));
    return SLOT_OTHER;
}

static unsigned long idForSlot(uint8_t s) {
    if (s < 18) return 0x0A0UL + s;
    if (s < 21) return 0x0C0UL + (s - 18);
    return otherId;
}

// J1939 mode: 0x0CFFxx01 -> same ID as the standard frame (0x0xx)
static unsigned long normalizeId(unsigned long id) {
    if (id > 0x7FFUL && (id & 0xFFFF00FFUL) == 0x0CFF0001UL) return (id >> 8) & 0xFF;
    return id;
}

// ---------------------------------------------------------------------
//  Frame intake: store the latest frame per ID, flag important changes
// ---------------------------------------------------------------------
static void handleFrame(const CANMessage &m) {
    unsigned long id = normalizeId(m.id);

    if (rawMode) {
        printRawCsv(id, m);
        return;
    }

    uint8_t s = slotForId(id);
    Slot &sl = slots[s];
    const uint8_t *d = m.data;
    const uint8_t *p = sl.data;

    bool urgent = (sl.len == 0);        // first time we see this ID
    if (!urgent && m.length == 8) {
        switch (id) {
            case 0x0AA:                 // VSM state, inverter state, mode, enable/lockout
                urgent = (d[0] != p[0]) || (d[2] != p[2]) ||
                         ((d[4] ^ p[4]) & 0x01) || (d[6] != p[6]);
                break;
            case 0x0AB:                 // any fault change
                for (uint8_t i = 0; i < 8; i++) if (d[i] != p[i]) urgent = true;
                break;
            case 0x0C0:                 // direction, enable bits, torque limit (not torque itself)
                urgent = (d[4] != p[4]) || (d[5] != p[5]) || (d[6] != p[6]) || (d[7] != p[7]);
                break;
            case 0x0C1:
            case 0x0C2:                 // parameter traffic is rare: show it all
                urgent = true;
                break;
            default:
                break;
        }
    }

    for (uint8_t i = 0; i < 8; i++) sl.data[i] = (i < m.length) ? d[i] : 0;
    sl.len = m.length ? m.length : 1;   // keep non-zero so "first frame" logic works
    sl.fresh = true;
    sl.urgent = sl.urgent || urgent;
    if (s == SLOT_OTHER) otherId = id;
}

// ---------------------------------------------------------------------
//  Print scheduler: at most ONE line per loop() pass so the MCP2515
//  RX buffers get drained between prints
// ---------------------------------------------------------------------
static void emitSlot(uint8_t i) {
    Slot &s = slots[i];
    printLine(idForSlot(i), s.data, s.len);
    s.fresh = false;
    s.urgent = false;
    s.lastPrint = millis();
}

static void printNext() {
    for (uint8_t i = 0; i < NUM_SLOTS; i++) {       // urgent first
        if (slots[i].fresh && slots[i].urgent) { emitSlot(i); return; }
    }
    unsigned long now = millis();
    for (uint8_t k = 0; k < NUM_SLOTS; k++) {       // then round-robin by age
        uint8_t i = (uint8_t)((rrNext + k) % NUM_SLOTS);
        if (slots[i].fresh && (now - slots[i].lastPrint >= intervalMs)) {
            emitSlot(i);
            rrNext = (uint8_t)((i + 1) % NUM_SLOTS);
            return;
        }
    }
}

// ---------------------------------------------------------------------
//  Serial commands
// ---------------------------------------------------------------------
static void printHelp() {
    Serial.println(F("# CAN sniffer commands: r = raw CSV on/off, f = print every frame on/off, h = help"));
    Serial.println(F("# decoded format: [ms] ID NAME fields... | raw bytes"));
}

static void handleSerial() {
    while (Serial.available()) {
        char c = (char)Serial.read();
        if (c == 'r') {
            rawMode = !rawMode;
            Serial.println(rawMode ? F("# raw CSV mode ON (ms,id,len,bytes)") : F("# raw CSV mode OFF"));
        } else if (c == 'f') {
            intervalMs = (intervalMs == 0) ? DEFAULT_INTERVAL_MS : 0;
            Serial.println(intervalMs == 0 ? F("# printing every frame") : F("# throttled to 1 line per ID per 100 ms"));
        } else if (c == 'h') {
            printHelp();
        }
    }
}

// ---------------------------------------------------------------------
//  Arduino entry points
// ---------------------------------------------------------------------
void setup() {
    setupSerialBlocking();
    Serial.println(F("# CAN sniffer starting (Uno + Seeed CAN shield V2)"));

    // Not using setupCANBlocking() so the baud rate is configurable above
    while (CAN.begin(SNIFFER_CAN_SPEED) != CAN_OK) {
        Serial.println(F("# CAN init failed (shield seated? CS=D9?) - retrying"));
        delay(500);
    }
#if SNIFFER_LISTEN_ONLY
    CAN.setMode(MODE_LISTENONLY);
    Serial.println(F("# listen-only mode"));
#endif

    Serial.println(F("# CAN init OK"));
    printHelp();

    lastRxMs = millis();
    lastStatsMs = millis();
}

void loop() {
    handleSerial();

    // 1) Drain pending frames from the MCP2515 (fast, no printing)
    CANMessage m;
    for (uint8_t n = 0; n < MAX_FRAMES_PER_PASS && getMsg(m); n++) {
        frameCount++;
        lastRxMs = millis();
        silentWarned = false;
        handleFrame(m);
    }

    // 2) Print at most one decoded line
    if (!rawMode) printNext();

    // 3) Housekeeping
    unsigned long now = millis();
    if (!silentWarned && (now - lastRxMs) > SILENCE_WARN_MS) {
        Serial.println(F("# no CAN traffic: check wiring, termination, power, and SNIFFER_CAN_SPEED"));
        silentWarned = true;
    }
    if (now - lastStatsMs >= STATS_PERIOD_MS) {
        Serial.print(F("# "));
        Serial.print(frameCount / (STATS_PERIOD_MS / 1000UL));
        Serial.println(F(" frames/s"));
        frameCount = 0;
        lastStatsMs = now;
    }
}