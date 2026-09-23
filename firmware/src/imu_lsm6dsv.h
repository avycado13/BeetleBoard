/**
 * @file imu_lsm6dsv.h
 * @brief SPI Driver for ST LSM6DSV320XTR 6-Axis High-G IMU on BeetleBoard
 */

#pragma once

#include <Arduino.h>
#include <SPI.h>
#include "beetleboard_pins.h"

// Register Addresses
#define LSM6DSV_REG_FUNC_CFG_ACCESS 0x01
#define LSM6DSV_REG_WHO_AM_I        0x0F
#define LSM6DSV_REG_CTRL1_XL        0x10
#define LSM6DSV_REG_CTRL2_G         0x11
#define LSM6DSV_REG_CTRL3           0x12
#define LSM6DSV_REG_STATUS_REG      0x1E
#define LSM6DSV_REG_OUT_TEMP_L      0x1D
#define LSM6DSV_REG_OUT_TEMP_H      0x1E
#define LSM6DSV_REG_OUTX_L_G        0x22
#define LSM6DSV_REG_OUTX_H_G        0x23
#define LSM6DSV_REG_OUTY_L_G        0x24
#define LSM6DSV_REG_OUTY_H_G        0x25
#define LSM6DSV_REG_OUTZ_L_G        0x26
#define LSM6DSV_REG_OUTZ_H_G        0x27
#define LSM6DSV_REG_OUTX_L_A        0x28
#define LSM6DSV_REG_OUTX_H_A        0x29
#define LSM6DSV_REG_OUTY_L_A        0x2A
#define LSM6DSV_REG_OUTY_H_A        0x2B
#define LSM6DSV_REG_OUTZ_L_A        0x2C
#define LSM6DSV_REG_OUTZ_H_A        0x2D

struct ImuRawData {
    int16_t accelX;
    int16_t accelY;
    int16_t accelZ;
    int16_t gyroX;
    int16_t gyroY;
    int16_t gyroZ;
    int16_t temperature;
};

struct ImuScaledData {
    float accelX_g;     // Acceleration in Gs (up to ±320G)
    float accelY_g;
    float accelZ_g;
    float gyroX_dps;    // Angular rate in degrees per second (up to ±4000 dps)
    float gyroY_dps;
    float gyroZ_dps;
    float temp_c;       // Temperature in Celsius
};

class ImuLsm6dsv {
public:
    ImuLsm6dsv();

    /**
     * @brief Initialize SPI bus and verify LSM6DSV320X communication.
     * @return true if device responded with valid WHO_AM_I (0x73).
     */
    bool begin();

    /**
     * @brief Read WHO_AM_I register (expected 0x73).
     */
    uint8_t readWhoAmI();

    /**
     * @brief Poll sensor and read latest 6-axis acceleration and gyro data.
     * @return true if new data was successfully read.
     */
    bool readData();

    /**
     * @brief Get latest raw 16-bit counts.
     */
    const ImuRawData& getRaw() const { return _raw; }

    /**
     * @brief Get latest scaled physical engineering units.
     */
    const ImuScaledData& getScaled() const { return _scaled; }

    /**
     * @brief Calculate net total acceleration magnitude in Gs.
     */
    float getTotalAccelG() const;

    /**
     * @brief Calculate yaw spin rate (Z-axis gyro) in RPM (useful for meltybrain bots).
     */
    float getYawRpm() const;

    /**
     * @brief Low-level register read.
     */
    uint8_t readRegister(uint8_t reg);

    /**
     * @brief Low-level register write.
     */
    void writeRegister(uint8_t reg, uint8_t value);

    /**
     * @brief Multi-byte burst read.
     */
    void readRegisters(uint8_t startReg, uint8_t* buffer, size_t length);

private:
    void initSensor();
    void computeScaled();

    SPIClass _spi;
    SPISettings _spiSettings;

    ImuRawData _raw = {};
    ImuScaledData _scaled = {};

    float _accelSensitivity = 0.000488f; // ±16g default: ~0.488 mg/LSB
    float _gyroSensitivity  = 0.070f;    // ±2000 dps default: ~70 mdps/LSB
    bool _initialized = false;
};
