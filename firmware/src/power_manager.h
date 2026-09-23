/**
 * @file power_manager.h
 * @brief Battery voltage monitoring and software Undervoltage Lockout (UVLO)
 *        for BeetleBoard's TPS563200 5V buck converter.
 */

#pragma once

#include <Arduino.h>
#include "beetleboard_pins.h"

enum class BatteryState {
    UNKNOWN,
    HEALTHY,
    LOW_WARNING,
    CRITICAL_UVLO
};

class PowerManager {
public:
    PowerManager();

    /**
     * @brief Initialize ADC and BUCK_EN control pin.
     * @param autoEnableBuck If true, pulls BUCK_EN high on startup.
     */
    void begin(bool autoEnableBuck = true);

    /**
     * @brief Periodically called in loop() to read ADC and evaluate UVLO.
     */
    void update();

    /**
     * @brief Explicitly enable or disable the TPS563200 5V buck regulator.
     */
    void setBuckEnabled(bool enable);

    /**
     * @brief Check whether 5V buck regulator is currently enabled.
     */
    bool isBuckEnabled() const { return _buckEnabled; }

    /**
     * @brief Get measured battery voltage (Volts).
     */
    float getBatteryVoltage() const { return _filteredVbat; }

    /**
     * @brief Get measured ADC pin voltage in millivolts.
     */
    uint32_t getPinMillivolts() const { return _pinMilliVolts; }

    /**
     * @brief Get detected LiPo cell count (e.g. 2, 3, or 4).
     */
    uint8_t getCellCount() const { return _cellCount; }

    /**
     * @brief Get average cell voltage in Volts.
     */
    float getCellVoltage() const;

    /**
     * @brief Get battery health state.
     */
    BatteryState getBatteryState() const { return _state; }

    /**
     * @brief Set custom cutoff voltage per cell (default is 3.3V).
     */
    void setCutoffPerCell(float volts) { _cutoffPerCell = volts; }

    /**
     * @brief Set software UVLO debounce time in ms (ignores transient motor sag).
     */
    void setSagFilterTimeMs(uint32_t ms) { _sagFilterTimeMs = ms; }

    /**
     * @brief Reset UVLO trip state after charging or connecting new battery.
     */
    void resetUvlo();

private:
    void sampleVoltage();
    void detectCellCount();
    void evaluateSafety();

    bool _buckEnabled = false;
    bool _uvloTripped = false;
    uint32_t _pinMilliVolts = 0;
    float _filteredVbat = 0.0f;
    uint8_t _cellCount = 0;

    float _cutoffPerCell = LIPO_CELL_MIN_VOLTAGE;
    float _warnPerCell   = LIPO_CELL_WARN_VOLTAGE;
    uint32_t _sagFilterTimeMs = 1500; // 1.5 seconds below threshold before cut
    uint32_t _lowVoltageStartTime = 0;
    uint32_t _lastSampleTime = 0;

    BatteryState _state = BatteryState::UNKNOWN;
};
