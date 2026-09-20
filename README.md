# BeetleBoard

Ever wanted to build a beetleweight but didn't want the huge mess of wires? No? Too bad. Meet BeetleBoard.
A perfect board for controlling your beetleweight battlebot using an ESP32.

![PCB](https://user-cdn.hackclub-assets.com/01a0a197-1997-7973-8d8f-44eda1789c8c/beetleboard-9.png)
![3D View](https://cdn.hackclub.com/01a0bf82-e3b2-7174-9dcd-bccaf0c52bd3/beetleboard-12.png)

## Features:
- 8 ports for ESCs
- 8 motor power ports
- Optional ExpressLRS Receiver
- 6 axis IMU with handling for up to 320Gs
- Software controllable battery undervoltage lockout circuitry
- Battery voltage sensing
- Powered by a XIAO ESP32-C6
- Ability to add a power switch (preferably fingertech)

## Why?
I built this because I saw battlebots at open sauce and thought "I want to make one of those". While looking through things about beetleweights and especially the ones with custom PCBs, I found out that most of the PCBs I found were specifically meant for one design and couldn't really be used for any type of bot. Coincidentally, My friend was trying to run a battlebots elective at our school and would need a control system to test before teaching the students.

## Bill of Materials (BOM)

| Qty | Reference | Value | Package | Description |
|----:|-----------|-------|---------|-------------|
| 1 | U1 | XIAO ESP32-C6 SMD | XIAO | Seeed Studio XIAO ESP32-C6 module (main MCU) |
| 1 | U2 | TPS563200 | SOT-23-6 | 3A synchronous step-down buck converter (5V rail) |
| 1 | U3 | LSM6DSV320XTR | LGA-14 | 6-axis IMU, up to 320G |
| 1 | L1 | 4.7µH | 1210 | Buck converter power inductor |
| 5 | C1, C2, C3, C6, C8 | 22µF | 0805 | Bulk decoupling / output capacitance |
| 5 | C4, C5, C7, C9, C10 | 100nF | 0805 | Decoupling capacitors |
| 9 | R1-R6, R11-R13 | 33kΩ | 0805 | Resistor |
| 1 | R7 | 56kΩ | 0805 | Resistor |
| 1 | R8 | 10kΩ | 0805 | Resistor |
| 1 | R9 | 33.2kΩ | 0805 | Resistor |
| 1 | R10 | 120kΩ | 0805 | Resistor |
| 1 | R14 | 100kΩ | 0805 | Resistor |
| 1 | D1 | SMAJ40CA | SMA | 40V TVS diode (battery reverse/over-voltage protection) |
| 8 | D2-D9 | PESD5V0V1BA | SOD-323 | 5V ESD protection diodes |
| 1 | J1 | AMASS XT60-F | XT60 | Battery connector |
| 8 | J2-J7, J16, J17 | ESC signal port | 1×3 pin header 2.54mm | ESC signal power ports A-H |
| 8 | J8-J15 | VBAT ESC port | Solder wire 2-pin | Motor power ports |
| 1 | J18 | ELRS Receiver | BetaFPV Lite | Optional ExpressLRS receiver socket |
| 1 | SW1 | FingerTech Switch | Solder wire 2-pin | External power switch connection |
| 5 | TP1-TP5 | Test point | 3mm pad | Test/debug points (VBAT, 5V, GND, 3.3V, VBUS) |
