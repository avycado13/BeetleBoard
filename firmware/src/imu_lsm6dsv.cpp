/**
 * @file imu_lsm6dsv.cpp
 * @brief Implementation of LSM6DSV320XTR 6-axis IMU driver over SPI
 */

#include "imu_lsm6dsv.h"
#include <math.h>

ImuLsm6dsv::ImuLsm6dsv()
    : _spi(FSPI), _spiSettings(IMU_SPI_CLOCK_HZ, MSBFIRST, SPI_MODE0) {}

bool ImuLsm6dsv::begin() {
    pinMode(PIN_IMU_CS, OUTPUT);
    digitalWrite(PIN_IMU_CS, HIGH);

    // Initialize SPI bus with BeetleBoard schematic pins:
    // SCK: 19, MISO: 20, MOSI: 18, CS: 23
    _spi.begin(PIN_IMU_SCK, PIN_IMU_MISO, PIN_IMU_MOSI, PIN_IMU_CS);

    delay(20);

    // Verify sensor identity
    uint8_t id = readWhoAmI();
    if (id != LSM6DSV320X_WHO_AM_I_VAL) {
        Serial.printf("[IMU] WHO_AM_I mismatch! Expected 0x%02X, got 0x%02X\n",
                      LSM6DSV320X_WHO_AM_I_VAL, id);
        return false;
    }

    initSensor();
    _initialized = true;
    Serial.printf("[IMU] LSM6DSV320X detected successfully (ID: 0x%02X)\n", id);
    return true;
}

uint8_t ImuLsm6dsv::readWhoAmI() {
    return readRegister(LSM6DSV_REG_WHO_AM_I);
}

void ImuLsm6dsv::initSensor() {
    // 1. Perform software reset
    writeRegister(LSM6DSV_REG_CTRL3, 0x01); // SW_RESET
    delay(15);

    // 2. Enable Block Data Update (BDU) and Auto-Increment (IF_INC)
    writeRegister(LSM6DSV_REG_CTRL3, 0x44); // BDU=1, IF_INC=1

    // 3. Configure Accelerometer (CTRL1_XL, 0x10):
    //    Bit [3:0] ODR = 0110b (120 Hz)
    //    Bit [5:4] FS  = 0011b (±16g full-scale for general combat, can be set to high-g mode)
    writeRegister(LSM6DSV_REG_CTRL1_XL, 0x36);
    _accelSensitivity = 0.000488f; // ±16g: ~0.488 mg/LSB -> 0.000488 g/LSB

    // 4. Configure Gyroscope (CTRL2_G, 0x11):
    //    Bit [3:0] ODR = 0110b (120 Hz)
    //    Bit [7:4] FS  = 0100b (±2000 dps full-scale)
    writeRegister(LSM6DSV_REG_CTRL2_G, 0x46);
    _gyroSensitivity = 0.070f; // ±2000 dps: 70 mdps/LSB -> 0.070 dps/LSB

    delay(20);
}

uint8_t ImuLsm6dsv::readRegister(uint8_t reg) {
    _spi.beginTransaction(_spiSettings);
    digitalWrite(PIN_IMU_CS, LOW);

    // Bit 7 = 1 for read in ST SPI protocol
    _spi.transfer(reg | 0x80);
    uint8_t val = _spi.transfer(0x00);

    digitalWrite(PIN_IMU_CS, HIGH);
    _spi.endTransaction();
    return val;
}

void ImuLsm6dsv::writeRegister(uint8_t reg, uint8_t value) {
    _spi.beginTransaction(_spiSettings);
    digitalWrite(PIN_IMU_CS, LOW);

    // Bit 7 = 0 for write
    _spi.transfer(reg & 0x7F);
    _spi.transfer(value);

    digitalWrite(PIN_IMU_CS, HIGH);
    _spi.endTransaction();
}

void ImuLsm6dsv::readRegisters(uint8_t startReg, uint8_t* buffer, size_t length) {
    _spi.beginTransaction(_spiSettings);
    digitalWrite(PIN_IMU_CS, LOW);

    _spi.transfer(startReg | 0x80);
    for (size_t i = 0; i < length; i++) {
        buffer[i] = _spi.transfer(0x00);
    }

    digitalWrite(PIN_IMU_CS, HIGH);
    _spi.endTransaction();
}

bool ImuLsm6dsv::readData() {
    if (!_initialized) return false;

    // Check data ready flags in STATUS_REG (0x1E)
    uint8_t status = readRegister(LSM6DSV_REG_STATUS_REG);
    bool accelReady = status & 0x01;
    bool gyroReady  = status & 0x02;

    if (!accelReady && !gyroReady) {
        return false;
    }

    // Burst read 12 registers from OUTX_L_G (0x22) to OUTZ_H_A (0x2D)
    uint8_t buf[12];
    readRegisters(LSM6DSV_REG_OUTX_L_G, buf, 12);

    // Gyroscope data: 0x22..0x27
    _raw.gyroX = (int16_t)((buf[1] << 8) | buf[0]);
    _raw.gyroY = (int16_t)((buf[3] << 8) | buf[2]);
    _raw.gyroZ = (int16_t)((buf[5] << 8) | buf[4]);

    // Accelerometer data: 0x28..0x2D
    _raw.accelX = (int16_t)((buf[7] << 8) | buf[6]);
    _raw.accelY = (int16_t)((buf[9] << 8) | buf[8]);
    _raw.accelZ = (int16_t)((buf[11] << 8) | buf[10]);

    computeScaled();
    return true;
}

void ImuLsm6dsv::computeScaled() {
    _scaled.accelX_g = (float)_raw.accelX * _accelSensitivity;
    _scaled.accelY_g = (float)_raw.accelY * _accelSensitivity;
    _scaled.accelZ_g = (float)_raw.accelZ * _accelSensitivity;

    _scaled.gyroX_dps = (float)_raw.gyroX * _gyroSensitivity;
    _scaled.gyroY_dps = (float)_raw.gyroY * _gyroSensitivity;
    _scaled.gyroZ_dps = (float)_raw.gyroZ * _gyroSensitivity;
}

float ImuLsm6dsv::getTotalAccelG() const {
    return sqrtf((_scaled.accelX_g * _scaled.accelX_g) +
                 (_scaled.accelY_g * _scaled.accelY_g) +
                 (_scaled.accelZ_g * _scaled.accelZ_g));
}

float ImuLsm6dsv::getYawRpm() const {
    // 360 degrees per second = 1 rev/sec = 60 RPM
    // RPM = dps * (60 / 360) = dps / 6.0
    return _scaled.gyroZ_dps / 6.0f;
}
