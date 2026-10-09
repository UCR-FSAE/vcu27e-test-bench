# vcu27e-test-bench
Test bench for vcu27E

## Project description
We are currently using an arduino uno R3 with a Seeed Studio CAN-Bus Shield V2 to test the functionality 3 three VCU commands:
* vcu sends the correct inverter initialization sequence
* correct torque command code
* canbus message functions

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

## UML Diagram
plan the classes/functions we will make for the project

## Test Details
define more precisely what the tests are supposed to do...

### Usage
* Open the PlatformIO Project Workspace
* Build Code (Project Tasks > General, click Build)
* plug in board and upload code to board(Project Tasks > General, click Upload)
* Open serial monitor (Project Tasks > General, click Monitor)

alternatively: 
* pio run -t upload
* pio device monitor -b 500000

## Additional links
[VCU27 GitHub](https://github.com/UCR-FSAE/vcu27E)

[VCU Documentation](https://docs.google.com/document/d/1MOWujTcB2fBFQVCMLC8CghEVTRkuquVf8tJmGK1droA/edit?usp=sharing)

[CAN Shield Library](https://github.com/Seeed-Studio/Seeed_Arduino_CAN)
