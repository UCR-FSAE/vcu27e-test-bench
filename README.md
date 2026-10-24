# vcu27e-test-bench
Test bench for vcu27E

## Project description
We are currently using an arduino uno R3 with a Seeed Studio CAN-Bus Shield V2 to test the functionality of the VCU.

## Hardware
* Arduino UNO R3
* Seeed Studio CAN-Bus Shield V2
* VCU runs on: teensy 4.1 (vcu27), NUCLEO-F756ZG (vcu26)
* Adafruit MCP4728 Quad DAC with EEPROM
* 5V to 3.3V level converter
* Potentiometers 

## Pinouts

<img width="400" height="314" alt="Can bus shield" src="https://github.com/user-attachments/assets/2a5a6887-5a05-4087-bbdd-e4445c919aea" />

<img width="230" height="314" alt="stm f7" src="https://github.com/user-attachments/assets/2b801e57-0d2c-433e-afcf-a170cdcf6c0c" />

| Signal | CAN Shield V2 | STM32 F756ZG |
|---|---|---|
| CAN_RX | CANL / CANH | PD0 |
| CAN_TX | CANL / CANH | PD1 |
| APPS1 | 3 | PA5 |
| APPS2 | 5 | PA6 |
| BSE | 6 | PF8 |
| Brake Light Output | A0 | PA4 |
| Buzzer Signal Output | A1 | PB11 |
| Driver Action | 7 | PB10 |
| Tractive System Active | 8 | PB9 |

<img width="340" height="314" alt="teensy4.1" src="https://github.com/user-attachments/assets/a4dd9b3f-cbcf-4613-998c-c900d3ae1294" />

| Signal | CAN Shield V2 | Teensy |
| :--- | :--- | :--- |
| CAN_RX | CANL / CANH | 23 |
| CAN_TX | CANL / CANH | 22 |
| APPS1 | 3 | 41 |
| APPS2 | 5 | 40|
| BSE | 6 | 39 |
| Brake Light Output | A0 | 15 |
| Buzzer Signal Output | A1 | 14 |
| Driver Action | 7 | 2 |
| Tractive System Active | 8 | 3 |
* dont put more than 3.3V into the Teensy

## Tests
We are testing our testbench with the vcu26(STM32 F756ZG) to check that the functionality of the vcu27(Teensy 4.1) is consistent with our previous vcu.

### RTD Test (in progress)
testbench sends the traction system active, driver input, brake pressed. check for correct buzzer timing

### Inverter Initialization Test
The testbench waits for the disabled command from the VCU and sends a disabled and locked command to the VCU. Once the testbench checks that the disable command is correct, it waits for the enable command from the VCU and sends a message to the VCU to signal it is disabled and unlocked. Once the testbench gets the enable command, it sends a message to the VCU to signal the inverter is enabled and unlocked.

```
Testbench -> VCU (CAN ID 0x0AA)
disabled + locked    0 0 0 0 0 0 0 0
disabled + unlocked  0 0 0 0 0 0 80 0
enabled + unlocked   0 0 0 0 0 0 1 0
VCU -> Testbench (CAN ID 0x0C0)
disable command      0 0 0 0 0 0 0 0
enable command       0 0 0 0 0 1 0 0
```

Serial Output:
```
Waiting for inverter initialization sequence...
C0 =  0 0 0 0 0 0 0 0
Got Disable CMD
Waiting for inverter initialization sequence...
C0 =  0 0 0 0 0 1 0 0
Got Enable CMD
Inverter Initialization Successful.
```

### APPS/BSE Test (in progress)

### Motor Spin Test
Checks that torque values are sent to the correct location on the can bus. The first two bytes are little endian and contain the torque value with byte 0 as the low half and byte 1 as the high half. The vcu sends a 1 on byte 5 to signal inverter enable.

Serial Output:
```
C0 = A0 0F 0 0 0 1 0 0
C0 = A0 0F 0 0 0 1 0 0
…
```

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
