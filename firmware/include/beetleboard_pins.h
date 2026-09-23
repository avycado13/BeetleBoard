/**
 * @file beetleboard_pins.h
 * @brief Pin definitions and board hardware configuration for BeetleBoard
 * 
 * Hardware: Seeed Studio XIAO ESP32-C6 SMD module
 * Project:  BeetleBoard (Avycado13)
 * Target:   Beetleweight Battlebot Controller
 *
 * Generated from BeetleBoard KiCad Schematics:
 * - BeetleBoard.kicad_sch
 * - power.kicad_sch
 * - imu.kicad_sch
 */

#pragma once

#include <stdint.h>

// =============================================================================
// XIAO ESP32-C6 Onboard Peripherals
// =============================================================================
// The onboard user LED is active-LOW on GPIO 15
#define PIN_LED_BUILTIN         15
#define LED_ACTIVE_LEVEL        LOW
#define LED_INACTIVE_LEVEL      HIGH

// =============================================================================
// Power & Battery Monitoring Circuitry (power.kicad_sch)
// =============================================================================
// VBAT Sensing: Voltage divider with R10 (120kΩ) and R9 (33.2kΩ), C7 (100nF) filter
// Connected to XIAO Pin D0 (ESP32-C6 GPIO 0 / ADC1_CH0)
#define PIN_VBAT_SNS            0

// Buck Converter Enable: Controls EN pin of TPS563200 3A 5V Buck Regulator (U2)
// Connected to XIAO Pin D1 (ESP32-C6 GPIO 1)
// Active-HIGH: Set HIGH to enable 5V rail; set LOW for software UVLO shutdown
#define PIN_BUCK_EN             1

// Resistor divider values for VBAT calculation:
// V_sns = V_bat * (R_BOTTOM / (R_TOP + R_BOTTOM))
// V_bat = V_sns * ((R_TOP + R_BOTTOM) / R_BOTTOM)
#define VBAT_DIVIDER_R_TOP      120000.0f   // 120 kΩ (R10)
#define VBAT_DIVIDER_R_BOTTOM   33200.0f    // 33.2 kΩ (R9)
#define VBAT_DIVIDER_RATIO      (VBAT_DIVIDER_R_BOTTOM / (VBAT_DIVIDER_R_TOP + VBAT_DIVIDER_R_BOTTOM)) // ~0.21671
#define VBAT_SCALE_FACTOR       ((VBAT_DIVIDER_R_TOP + VBAT_DIVIDER_R_BOTTOM) / VBAT_DIVIDER_R_BOTTOM) // ~4.61446

// =============================================================================
// ESC / Motor Signal Ports (BeetleBoard.kicad_sch)
// =============================================================================
// 6 ESC ports connected directly to ESP32-C6 GPIOs with ESD protection (D2-D7)
// and 33k pull-downs (R1-R6). Connectors are standard 3-pin 0.1" (Signal, 5V, GND).
#define NUM_ESC_CHANNELS        6

#define PIN_ESC_A               2    // J2: XIAO D2 (GPIO 2)
#define PIN_ESC_B               21   // J3: XIAO D3 (GPIO 21)
#define PIN_ESC_C               22   // J4: XIAO D4 (GPIO 22)
#define PIN_ESC_D               5    // J5: XIAO MTDI (GPIO 5)
#define PIN_ESC_E               7    // J6: XIAO MTDO (GPIO 7)
#define PIN_ESC_F               6    // J7: XIAO MTCK (GPIO 6)

// Note: Ports ESC_G (J16) and ESC_H (J17) exist on the PCB with ESD protection
// and pull-downs, but their signal pins are unrouted to the MCU on this revision.
#define PIN_ESC_G_CONNECTED     0
#define PIN_ESC_H_CONNECTED     0

// Array of active ESC pins for easy iteration
static const uint8_t ESC_PINS[NUM_ESC_CHANNELS] = {
    PIN_ESC_A,  // Channel 0 (ESC A)
    PIN_ESC_B,  // Channel 1 (ESC B)
    PIN_ESC_C,  // Channel 2 (ESC C)
    PIN_ESC_D,  // Channel 3 (ESC D)
    PIN_ESC_E,  // Channel 4 (ESC E)
    PIN_ESC_F   // Channel 5 (ESC F)
};

// =============================================================================
// 6-Axis High-G IMU: LSM6DSV320XTR (imu.kicad_sch)
// =============================================================================
// Communicates over dedicated 4-wire SPI bus (3.3V logic)
#define PIN_IMU_CS              23   // U3 Pin 12: XIAO D5 (GPIO 23)
#define PIN_IMU_SCK             19   // U3 Pin 13: XIAO D8 (GPIO 19, R14 100k pulldown)
#define PIN_IMU_MISO            20   // U3 Pin 1:  XIAO D9 (GPIO 20, R13 33k pullup)
#define PIN_IMU_MOSI            18   // U3 Pin 14: XIAO D10 (GPIO 18)

// LSM6DSV320X Device ID & SPI Speeds
#define LSM6DSV320X_WHO_AM_I_VAL 0x73
#define IMU_SPI_CLOCK_HZ        4000000 // 4 MHz SPI clock

// =============================================================================
// ExpressLRS (ELRS) / CRSF Receiver Port (BeetleBoard.kicad_sch)
// =============================================================================
// J18: 4-pin header for BetaFPV Lite / ExpressLRS receivers (TX, RX, 5V, GND)
#define PIN_ELRS_TX             16   // J18 Pin 1: XIAO D6 (GPIO 16) -> Receiver RX
#define PIN_ELRS_RX             17   // J18 Pin 2: XIAO D7 (GPIO 17) <- Receiver TX
#define ELRS_CRSF_BAUDRATE      420000  // Standard CRSF baud rate
#define ELRS_SERIAL_PORT        Serial1

// =============================================================================
// Safety & Battery Cutoff Defaults
// =============================================================================
#define LIPO_CELL_MIN_VOLTAGE   3.30f   // Cutoff threshold per cell (V)
#define LIPO_CELL_WARN_VOLTAGE  3.50f   // Warning threshold per cell (V)
#define LIPO_CELL_MAX_VOLTAGE   4.25f   // Overvoltage detection threshold per cell (V)
#define UVLO_HYSTERESIS_VOLTAGE 0.50f   // Voltage rise needed before re-enabling buck

// Default PWM timings (standard 50 Hz RC PWM, in microseconds)
#define PWM_FREQUENCY_HZ        50
#define PWM_PULSE_MIN_US        1000    // Full reverse / minimum throttle
#define PWM_PULSE_NEUTRAL_US    1500    // Neutral / stop for bidirectional ESCs
#define PWM_PULSE_MAX_US        2000    // Full forward / maximum throttle
#define PWM_DISARM_US           1000    // Disarmed / safe pulse
