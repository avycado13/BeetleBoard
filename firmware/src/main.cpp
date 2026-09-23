/**
 * @file main.cpp
 * @brief BeetleBoard Primary Example Firmware
 * 
 * Hardware: BeetleBoard (Seeed Studio XIAO ESP32-C6)
 * Author:   avycado13
 * Features:
 *   - Battery voltage monitoring & software UVLO (TPS563200 Buck Enable control)
 *   - 6-channel ESC / Servo PWM generator (ESCs A-F)
 *   - 6-axis High-G IMU driver over SPI (LSM6DSV320XTR)
 *   - ExpressLRS (ELRS) / CRSF serial receiver decoding (UART)
 *   - Interactive Serial CLI via USB for bench testing & calibration
 *   - Multi-layer safety failsafes (UVLO shutdown, RC loss timeout, arming logic)
 */

#include <Arduino.h>
#include "beetleboard_pins.h"
#include "power_manager.h"
#include "esc_driver.h"
#include "imu_lsm6dsv.h"
#include "elrs_receiver.h"

// Subsystem instances
PowerManager powerMgr;
EscDriver    escDriver;
ImuLsm6dsv   imu;
ElrsReceiver elrs;

// Firmware operational modes
enum class TelemetryMode {
    NONE,
    IMU_STREAM,
    RC_STREAM,
    BATTERY_STREAM
};

static TelemetryMode currentTelemetry = TelemetryMode::NONE;
static uint32_t lastLedToggle = 0;
static uint32_t lastTelemetryPrint = 0;
static bool ledState = false;

// Function prototypes
void printBanner();
void printHelp();
void printStatus();
void handleSerialCommands();
void updateLedStatus();
void processRcControl();
void runMotorBenchTest(uint8_t channel);

void setup() {
    // 1. Initialize USB CDC Serial
    Serial.begin(115200);
    delay(1000); // Allow USB serial to enumerate

    // 2. Configure status LED
    pinMode(PIN_LED_BUILTIN, OUTPUT);
    digitalWrite(PIN_LED_BUILTIN, LED_INACTIVE_LEVEL);

    printBanner();

    // 3. Initialize Power Management (ADC & BUCK_EN)
    Serial.println("[INIT] Initializing Power Manager & 5V Buck Rail...");
    powerMgr.begin(true); // Enable 5V buck converter by default
    Serial.printf("       Battery: %.2fV (%dS LiPo, %.2fV/cell)\n",
                  powerMgr.getBatteryVoltage(),
                  powerMgr.getCellCount(),
                  powerMgr.getCellVoltage());

    // 4. Initialize ESC PWM Driver
    Serial.println("[INIT] Initializing 6-Channel ESC Outputs (ESCs A-F)...");
    escDriver.begin(PWM_FREQUENCY_HZ);
    escDriver.setArmed(false); // Disarmed by default for safety
    Serial.println("       ESCs initialized in DISARMED state (Safe pulse: 1000us).");

    // 5. Initialize LSM6DSV320X 6-Axis IMU over SPI
    Serial.println("[INIT] Initializing LSM6DSV320XTR 6-Axis IMU via SPI...");
    if (imu.begin()) {
        Serial.println("       IMU initialized successfully (ODR: 120Hz, ±16G / ±2000 dps).");
    } else {
        Serial.println("       [WARN] LSM6DSV320X IMU not detected! Check soldering/power.");
    }

    // 6. Initialize ExpressLRS Receiver via Serial1
    Serial.println("[INIT] Initializing ExpressLRS / CRSF Receiver Port...");
    elrs.begin(ELRS_CRSF_BAUDRATE);
    Serial.printf("       Listening on UART (RX: GPIO %d, TX: GPIO %d @ 420kBaud).\n",
                  PIN_ELRS_RX, PIN_ELRS_TX);

    Serial.println("\n[READY] BeetleBoard system ready. Type '?' or 'h' for command menu.\n");
}

void loop() {
    // 1. Update subsystem states
    powerMgr.update();
    imu.readData();
    elrs.update();

    // 2. Process RC commands or failsafes
    processRcControl();

    // 3. Handle interactive USB serial CLI
    handleSerialCommands();

    // 4. Update status LED blink pattern
    updateLedStatus();
}

/**
 * @brief Robot control loop: maps ELRS sticks to ESCs with safety interlocks.
 */
void processRcControl() {
    // If UVLO has triggered, immediately force disarm
    if (powerMgr.getBatteryState() == BatteryState::CRITICAL_UVLO) {
        if (escDriver.isArmed()) {
            escDriver.setArmed(false);
            Serial.println("[SAFETY] Critical battery undervoltage! Motors disarmed.");
        }
        return;
    }

    // Check ELRS connection
    if (elrs.isConnected()) {
        bool armSwitch = elrs.isArmSwitchActive(4); // AUX1 switch

        if (armSwitch && !escDriver.isArmed()) {
            escDriver.setArmed(true);
            Serial.println("[RC] Transmitter armed motors (AUX1 HIGH).");
        } else if (!armSwitch && escDriver.isArmed()) {
            escDriver.setArmed(false);
            Serial.println("[RC] Transmitter disarmed motors (AUX1 LOW).");
        }

        if (escDriver.isArmed()) {
            // Channel mapping:
            // Ch 0 (Roll/Steering): -1.0 to +1.0
            // Ch 1 (Pitch/Throttle): -1.0 to +1.0
            // Ch 2 (Weapon / Throttle): 0.0 to 1.0
            float steering = elrs.getChannelBidirectional(0);
            float throttle = elrs.getChannelBidirectional(1);
            float weapon   = elrs.getChannelUnidirectional(2);

            // Arcade / Differential drive mixing for ESC A (Left) & ESC B (Right)
            float leftMotor  = throttle + steering;
            float rightMotor = throttle - steering;

            // Clamp drive values
            if (leftMotor > 1.0f) leftMotor = 1.0f;
            if (leftMotor < -1.0f) leftMotor = -1.0f;
            if (rightMotor > 1.0f) rightMotor = 1.0f;
            if (rightMotor < -1.0f) rightMotor = -1.0f;

            // Drive motors on ESC A and ESC B
            escDriver.setBidirectional(static_cast<uint8_t>(EscChannel::ESC_A), leftMotor);
            escDriver.setBidirectional(static_cast<uint8_t>(EscChannel::ESC_B), rightMotor);

            // Weapon motor on ESC C (unidirectional 0.0 to 1.0)
            escDriver.setUnidirectional(static_cast<uint8_t>(EscChannel::ESC_C), weapon);

            // Auxiliary channels on ESC D, E, F
            escDriver.setPulseUs(static_cast<uint8_t>(EscChannel::ESC_D), elrs.getChannelUs(5));
            escDriver.setPulseUs(static_cast<uint8_t>(EscChannel::ESC_E), elrs.getChannelUs(6));
            escDriver.setPulseUs(static_cast<uint8_t>(EscChannel::ESC_F), elrs.getChannelUs(7));
        }
    } else {
        // Receiver disconnected or lost signal -> failsafe
        if (escDriver.isArmed()) {
            escDriver.setArmed(false);
            Serial.println("[SAFETY] ELRS RC signal lost! Failsafe triggered: Disarmed.");
        }
    }
}

/**
 * @brief Status LED blink patterns indicating health and safety status.
 */
void updateLedStatus() {
    uint32_t now = millis();
    uint32_t interval = 1000;

    if (powerMgr.getBatteryState() == BatteryState::CRITICAL_UVLO) {
        interval = 100; // Rapid emergency strobe
    } else if (powerMgr.getBatteryState() == BatteryState::LOW_WARNING) {
        interval = 250; // Fast warning blink
    } else if (escDriver.isArmed()) {
        // Solid ON when armed
        digitalWrite(PIN_LED_BUILTIN, LED_ACTIVE_LEVEL);
        return;
    } else {
        // Disarmed standby: gentle heartbeat
        interval = elrs.isConnected() ? 500 : 1000;
    }

    if (now - lastLedToggle >= interval) {
        lastLedToggle = now;
        ledState = !ledState;
        digitalWrite(PIN_LED_BUILTIN, ledState ? LED_ACTIVE_LEVEL : LED_INACTIVE_LEVEL);
    }
}

/**
 * @brief Handle user commands from the USB Serial console.
 */
void handleSerialCommands() {
    if (Serial.available()) {
        char c = Serial.read();

        // If streaming telemetry, any key stops it
        if (currentTelemetry != TelemetryMode::NONE) {
            currentTelemetry = TelemetryMode::NONE;
            Serial.println("\n[INFO] Telemetry stream stopped.");
            return;
        }

        switch (c) {
            case 'h':
            case '?':
                printHelp();
                break;

            case 's':
                printStatus();
                break;

            case 'b': {
                bool newState = !powerMgr.isBuckEnabled();
                powerMgr.setBuckEnabled(newState);
                Serial.printf("[POWER] 5V Buck Regulator manually set to: %s\n",
                              newState ? "ENABLED (HIGH)" : "DISABLED (LOW)");
                break;
            }

            case 'a': {
                bool newArmed = !escDriver.isArmed();
                escDriver.setArmed(newArmed);
                Serial.printf("[ESC] Software Arming: %s\n",
                              newArmed ? "ARMED (CAUTION: Motors Live!)" : "DISARMED (Safe)");
                break;
            }

            case 'm':
                runMotorBenchTest(0); // Test ESC A
                break;

            case 't':
                currentTelemetry = TelemetryMode::IMU_STREAM;
                Serial.println("[STREAM] Streaming IMU data (press any key to stop)...");
                Serial.println("Time(ms) | Accel_X(g) | Accel_Y(g) | Accel_Z(g) | Gyro_X(dps) | Gyro_Y(dps) | Gyro_Z(dps) | Yaw(RPM)");
                break;

            case 'r':
                currentTelemetry = TelemetryMode::RC_STREAM;
                Serial.println("[STREAM] Streaming ELRS RC Channels (press any key to stop)...");
                break;

            case 'v':
                currentTelemetry = TelemetryMode::BATTERY_STREAM;
                Serial.println("[STREAM] Streaming Battery Voltage (press any key to stop)...");
                break;

            case 'u':
                powerMgr.resetUvlo();
                Serial.println("[POWER] UVLO state cleared, 5V buck re-enabled.");
                break;

            default:
                break;
        }
    }

    // Process continuous telemetry output if active
    if (currentTelemetry != TelemetryMode::NONE && (millis() - lastTelemetryPrint >= 100)) {
        lastTelemetryPrint = millis();

        if (currentTelemetry == TelemetryMode::IMU_STREAM) {
            const auto& s = imu.getScaled();
            Serial.printf("%8lu | %+10.2f | %+10.2f | %+10.2f | %+11.1f | %+11.1f | %+11.1f | %+8.1f\n",
                          millis(), s.accelX_g, s.accelY_g, s.accelZ_g,
                          s.gyroX_dps, s.gyroY_dps, s.gyroZ_dps,
                          imu.getYawRpm());
        } else if (currentTelemetry == TelemetryMode::RC_STREAM) {
            if (elrs.isConnected()) {
                Serial.printf("Ch1:%4d Ch2:%4d Ch3:%4d Ch4:%4d AUX1:%4d | LQ:%3d%% RSSI:-%ddBm\n",
                              elrs.getChannelUs(0), elrs.getChannelUs(1),
                              elrs.getChannelUs(2), elrs.getChannelUs(3),
                              elrs.getChannelUs(4),
                              elrs.getLinkStats().uplinkLinkQuality,
                              elrs.getLinkStats().uplinkRssi1);
            } else {
                Serial.println("No ELRS signal received...");
            }
        } else if (currentTelemetry == TelemetryMode::BATTERY_STREAM) {
            Serial.printf("Vbat: %.2fV (Pin: %umV) | Cell: %.2fV (%dS) | State: %s\n",
                          powerMgr.getBatteryVoltage(),
                          powerMgr.getPinMillivolts(),
                          powerMgr.getCellVoltage(),
                          powerMgr.getCellCount(),
                          powerMgr.getBatteryState() == BatteryState::HEALTHY ? "OK" :
                          powerMgr.getBatteryState() == BatteryState::LOW_WARNING ? "LOW" :
                          powerMgr.getBatteryState() == BatteryState::CRITICAL_UVLO ? "CRITICAL" : "UNKNOWN");
        }
    }
}

/**
 * @brief Safe bench motor sweep test for testing wiring and direction.
 */
void runMotorBenchTest(uint8_t channel) {
    Serial.println("\n---------------------------------------------------------");
    Serial.printf("[TEST] Running 3-second motor bench sweep on ESC %c (GPIO %d)\n",
                  'A' + channel, escDriver.getPin(channel));
    Serial.println("       WARNING: Ensure robot is securely propped up and clear!");
    Serial.println("---------------------------------------------------------");

    escDriver.setArmed(true);

    // Ramp up from 1000us to 1250us (low safe throttle)
    for (uint16_t pulse = 1000; pulse <= 1250; pulse += 10) {
        escDriver.setPulseUs(channel, pulse);
        delay(40);
    }
    delay(500);

    // Ramp back down to 1000us
    for (uint16_t pulse = 1250; pulse >= 1000; pulse -= 10) {
        escDriver.setPulseUs(channel, pulse);
        delay(40);
    }

    escDriver.setArmed(false);
    Serial.printf("[TEST] Motor test on ESC %c complete. Disarmed.\n\n", 'A' + channel);
}

void printBanner() {
    Serial.println("\n=======================================================");
    Serial.println("        ____            __  __     ____                         __ ");
    Serial.println("       / __ )___  ___  / /_/ /__  / __ )____  ____ _________   / / ");
    Serial.println("      / __  / _ \\/ _ \\/ __/ / _ \\/ __  / __ \\/ __ `/ ___/ __  / /  ");
    Serial.println("     / /_/ /  __/  __/ /_/ /  __/ /_/ / /_/ / /_/ / /  / /_/ /_/   ");
    Serial.println("    /_____/\\___/\\___/\\__/_/\\___/_____/\\____/\\__,_/_/   \\__,_(_)    ");
    Serial.println("          Beetleweight Battlebot Firmware Example v1.0         ");
    Serial.println("=======================================================");
}

void printHelp() {
    Serial.println("\n--- BeetleBoard Interactive Console ---");
    Serial.println("  s : Print comprehensive system status");
    Serial.println("  b : Toggle 5V Buck Regulator (BUCK_EN GPIO 1)");
    Serial.println("  a : Toggle Software Arming state");
    Serial.println("  m : Run bench motor ramp test on ESC A");
    Serial.println("  t : Stream live 6-axis IMU telemetry");
    Serial.println("  r : Stream live ExpressLRS RC channels");
    Serial.println("  v : Stream live battery voltage & UVLO status");
    Serial.println("  u : Clear UVLO trip and re-enable 5V buck");
    Serial.println("  h / ? : Display this help menu");
    Serial.println("---------------------------------------\n");
}

void printStatus() {
    Serial.println("\n================ BeetleBoard System Status ================");
    
    // Power subsystem
    Serial.println("[POWER]");
    Serial.printf("  Battery Voltage:    %.2f V (Pin reading: %u mV)\n",
                  powerMgr.getBatteryVoltage(), powerMgr.getPinMillivolts());
    Serial.printf("  Detected Battery:   %dS LiPo (Average cell: %.2f V)\n",
                  powerMgr.getCellCount(), powerMgr.getCellVoltage());
    Serial.printf("  TPS563200 5V Buck:  %s\n",
                  powerMgr.isBuckEnabled() ? "ENABLED (5V rail live)" : "DISABLED (UVLO cutoff)");
    Serial.printf("  Battery Health:     %s\n",
                  powerMgr.getBatteryState() == BatteryState::HEALTHY ? "HEALTHY" :
                  powerMgr.getBatteryState() == BatteryState::LOW_WARNING ? "LOW WARNING" :
                  powerMgr.getBatteryState() == BatteryState::CRITICAL_UVLO ? "CRITICAL (UVLO TRIPPED)" : "USB POWER / UNKNOWN");

    // ESC subsystem
    Serial.println("[ESC OUTPUTS]");
    Serial.printf("  Arming Status:      %s\n", escDriver.isArmed() ? "ARMED (LIVE)" : "DISARMED (SAFE)");
    for (uint8_t i = 0; i < NUM_ESC_CHANNELS; i++) {
        Serial.printf("  ESC %c (GPIO %2d):   %4d us\n",
                      'A' + i, escDriver.getPin(i), escDriver.getPulseUs(i));
    }

    // IMU subsystem
    Serial.println("[LSM6DSV320X IMU]");
    const auto& imuData = imu.getScaled();
    Serial.printf("  Accel (g):          X:%+6.2f  Y:%+6.2f  Z:%+6.2f  (Total: %.2f g)\n",
                  imuData.accelX_g, imuData.accelY_g, imuData.accelZ_g, imu.getTotalAccelG());
    Serial.printf("  Gyro (dps):         X:%+7.1f Y:%+7.1f Z:%+7.1f (Yaw RPM: %.1f)\n",
                  imuData.gyroX_dps, imuData.gyroY_dps, imuData.gyroZ_dps, imu.getYawRpm());

    // Receiver subsystem
    Serial.println("[EXPRESSLRS / CRSF]");
    Serial.printf("  Receiver Status:    %s\n", elrs.isConnected() ? "CONNECTED" : "NO SIGNAL / DISCONNECTED");
    if (elrs.isConnected()) {
        const auto& stats = elrs.getLinkStats();
        Serial.printf("  Link Quality:       %d %%\n", stats.uplinkLinkQuality);
        Serial.printf("  Uplink RSSI:        -%d dBm\n", stats.uplinkRssi1);
        Serial.printf("  Ch1-Ch4:            %d, %d, %d, %d us\n",
                      elrs.getChannelUs(0), elrs.getChannelUs(1),
                      elrs.getChannelUs(2), elrs.getChannelUs(3));
        Serial.printf("  Arm Switch (AUX1):  %s\n", elrs.isArmSwitchActive(4) ? "HIGH (ARM)" : "LOW (DISARM)");
    }

    Serial.println("===========================================================\n");
}
