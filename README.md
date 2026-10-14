# vcu27e-test-bench
Test bench for vcu27E

## Project description
We are currently using an arduino uno R3 with a Seeed Studio CAN-Bus Shield V2 to test the functionality of the VCU.

## Test Details

RTD sequence (in progress)

## Hardware
* Arduino UNO R3
* Seeed Studio CAN-Bus Shield V2
* VCU runs on: teensy 4.1 (vcu27), NUCLEO-F756ZG (vcu26)
* Inverter: 
* CAN module: 

## Arduino (CAN Shield V2) to VCU
| Signal | CAN Shield V2 | Teensy |
| :--- | :--- | :--- |
| CAN_RX | CANL / CANH | CANL / CANH |
| CAN_TX | CANL / CANH | CANL / CANH |
| APPS1 | 3 | 41 |
| APPS2 | 5 | 40|
| BSE | 6 | 39 |
| Brake Light Output | A0 | 15 |
| Buzzer Signal Output | A1 | 14 |
| Driver Action | 7 | 2 |
| Tractive System Active | 8 | 3 |
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

#### How to run tests:
...


#### Expected output of loop:
the test bench has a loop to print all the inputs recived from the vcu. 

#### How to read the CAN VCU-->INVERTER (0x0C0)
The vcu26 sends torque command messages on bytes 0 and 1 of 0x0C0 on the can bus.

The test bench prints all 8 bytes in hexadecimal and prints the torque value in decimal:

CAN rx 0x0C0 = A0 0F 00 00 00 01 00 00 torque=4000

A0 0F is little endian with byte 0 as the low half and byte 1 as the high half. The vcu sends a 1 on byte 5 to siginal inverter enable.

vcu26 sends torque command to the inverter as shown below:
```
/* Pack torque command (little-endian) */
txData[0] = (uint8_t)(torque & 0xFF);
txData[1] = (uint8_t)((torque >> 8) & 0xFF);

/* Speed command (0 for torque control mode) */
txData[2] = 0;
txData[3] = 0;

/* Direction and inverter control */
txData[4] = direction;
txData[5] = inverterEnable;

/* Torque limits (using default) */
txData[6] = 0;
txData[7] = 0;
```
## Additional links
[VCU27 GitHub](https://github.com/UCR-FSAE/vcu27E)

[VCU Documentation](https://docs.google.com/document/d/1MOWujTcB2fBFQVCMLC8CghEVTRkuquVf8tJmGK1droA/edit?usp=sharing)

[CAN Shield Library](https://github.com/Seeed-Studio/Seeed_Arduino_CAN)
