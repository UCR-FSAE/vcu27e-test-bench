# vcu27e-test-bench
Test bench for vcu27E

## Project description
We are currently using an arduino uno R3 with a Seeed Studio CAN-Bus Shield V2 to test the functionality of the VCU.

## Test Details

#### RTD Sequence

1) RTD sequence: inject Driver Action + Tractive System Active + BSE(transition to INVERTER_INIT, read buzzer time)
2) RTD interrupt: stop brake during RTD buzzer (no RTD)

#### Inverter

3) Inverter Handshake Success: complete RTD then send READY
4) Inverter Startup Timeout Gaurd: complete RTD then send no response
5) Inverter Startup Fault Check: complete RTD then send FAULT
6) Inverter CAN Signinal Loss: complete RTD then stop sending inverter messages (vcu enters fault within ?ms)

#### Torque

7) Torque no Drive: torque stays at zero when not DRIVE
8) Linear Pedal-to-torque Mapping: complete DRIVE then test varying APPS pedal voltage from 0-100% (VCU torque is proportional)
9) Brake-Throttle Interlock Cutoff: while in DRIVE at > 25% throttle, press brake pedal (VCU overrides input and forces commanded torque to 0)
10) APPS Implausibility: while in DRIVE inject >10% difference beterrn APPS1 and APPS2 (VCU sets torque to 0 and transitions to FAULT)
11) Tractive System Disable CAN Check: enter DRIVE, then disable the tractive system(VCU detects the loss of tractive system active status and transitions to FAULT, with commanded torque set to 0)

#### Can Communication

12) CAN transmission Loop Consistency: enter DRIVE, then measure for consistent frames (ex. 100 frames at 10±1ms)


## Hardware
* Arduino UNO R3
* Seeed Studio CAN-Bus Shield V2
* VCU runs on: teensy 4.1

## Arduino (CAN Shield V2) to VCU
| Signal | CAN Shield V2 | Teensy |
| :--- | :--- | :--- |
| CAN_RX | TBD | 23 |
| CAN_TX | TBD | 22 |
| APPS1 | TBD | 41 |
| APPS2 | TBD | 40|
| BSE | TBD | 39 |
| Brake Light Output | TBD | 15 |
| Buzzer Signal Output | TBD | 14 |
| Driver Action | TBD | 2 |
| Tractive System Active | TBD | 3 |
| GND | GND | GND |
* dont put more than 3.3V into the Teensy

### VCU Pinouts
| VCU actions: | Teensy | Config |
| :--- | :---: | :--- |
| CAN_RX | 23 | CAN_RX |
| CAN_TX | 22 | CAN_TX |
| APPS1 | 41 | ADC IN |
| APPS2 | 40 | ADC IN |
| BSE | 39 | ADC IN |
| Brake Light Output | 15 | GPIO OUT |
| Buzzer Signal Output | 14 | GPIO OUT |
| Driver Action | 2 | GPIO IN |
| Tractive System Active | 3 | GPIO IN |

## Installation/Usage
### Requirements
* VSCode
* Git
* PlatformIO (installed through vscode)

### Usage
* Open the PlatformIO Project Workspace
* Build Code (Project Tasks > General, click Build)
* plug in board and upload code to board(Project Tasks > General, click Upload)
* Open serial monitor (Project Tasks > General, click Monitor)

## Additional links
[VCU27 GitHub](https://github.com/UCR-FSAE/vcu27E)

[VCU Documentation](https://docs.google.com/document/d/1MOWujTcB2fBFQVCMLC8CghEVTRkuquVf8tJmGK1droA/edit?usp=sharing)

[CAN Shield Library](https://github.com/Seeed-Studio/Seeed_Arduino_CAN)
