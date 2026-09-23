/**
 * @file power_manager.cpp
 * @brief Implementation of battery monitoring and UVLO for BeetleBoard
 */

#include "power_manager.h"

PowerManager::PowerManager() {}

void PowerManager::begin(bool autoEnableBuck) {
    // Configure BUCK_EN pin (GPIO 1) as output
    pinMode(PIN_BUCK_EN, OUTPUT);

    // Default buck state
    setBuckEnabled(autoEnableBuck);

    // Configure ADC on PIN_VBAT_SNS (GPIO 0)
    pinMode(PIN_VBAT_SNS, INPUT);
    analogSetPinAttenuation(PIN_VBAT_SNS, ADC_11db);

    // Initial burst read to prime the filter
    for (int i = 0; i < 16; i++) {
        sampleVoltage();
        delay(2);
    }

    detectCellCount();
}

void PowerManager::sampleVoltage() {
    // Read calibrated millivolts directly from ESP32 ADC
    uint32_t mv = analogReadMilliVolts(PIN_VBAT_SNS);
    _pinMilliVolts = mv;

    // Convert to actual battery voltage via divider factor
    float measuredVbat = ((float)mv / 1000.0f) * VBAT_SCALE_FACTOR;

    // Exponential moving average filter (alpha = 0.15)
    if (_filteredVbat <= 0.1f) {
        _filteredVbat = measuredVbat;
    } else {
        _filteredVbat = (_filteredVbat * 0.85f) + (measuredVbat * 0.15f);
    }
}

void PowerManager::detectCellCount() {
    if (_filteredVbat > 13.0f) {
        _cellCount = 4; // 4S LiPo (14.8V nominal, 16.8V max)
    } else if (_filteredVbat > 8.8f) {
        _cellCount = 3; // 3S LiPo (11.1V nominal, 12.6V max) - standard beetleweight
    } else if (_filteredVbat > 5.5f) {
        _cellCount = 2; // 2S LiPo (7.4V nominal, 8.4V max)
    } else {
        _cellCount = 0; // Unplugged / USB powered only
    }
}

float PowerManager::getCellVoltage() const {
    if (_cellCount == 0) return 0.0f;
    return _filteredVbat / (float)_cellCount;
}

void PowerManager::setBuckEnabled(bool enable) {
    _buckEnabled = enable;
    digitalWrite(PIN_BUCK_EN, _buckEnabled ? HIGH : LOW);
}

void PowerManager::resetUvlo() {
    _uvloTripped = false;
    _lowVoltageStartTime = 0;
    setBuckEnabled(true);
}

void PowerManager::update() {
    uint32_t now = millis();
    if (now - _lastSampleTime < 50) {
        return; // Sample at 20 Hz
    }
    _lastSampleTime = now;

    sampleVoltage();

    if (_cellCount == 0 && _filteredVbat > 5.5f) {
        detectCellCount();
    }

    evaluateSafety();
}

void PowerManager::evaluateSafety() {
    // If running on USB bench power (Vbat near 0), don't trigger UVLO
    if (_filteredVbat < 4.0f || _cellCount == 0) {
        _state = BatteryState::UNKNOWN;
        return;
    }

    float cellV = getCellVoltage();

    if (cellV >= _warnPerCell) {
        _state = BatteryState::HEALTHY;
        _lowVoltageStartTime = 0;
    } else if (cellV > _cutoffPerCell) {
        _state = BatteryState::LOW_WARNING;
        _lowVoltageStartTime = 0;
    } else {
        // Voltage is below critical cutoff per cell
        if (_lowVoltageStartTime == 0) {
            _lowVoltageStartTime = millis();
        } else if (millis() - _lowVoltageStartTime >= _sagFilterTimeMs) {
            // Sustained undervoltage detected -> trigger UVLO
            _state = BatteryState::CRITICAL_UVLO;
            if (_buckEnabled && !_uvloTripped) {
                _uvloTripped = true;
                setBuckEnabled(false); // Shutdown 5V buck regulator
                Serial.printf("\n[ALERT] UVLO TRIGGERED! Vbat=%.2fV (%.2fV/cell). 5V Rail Disabled.\n",
                              _filteredVbat, cellV);
            }
        }
    }
}
