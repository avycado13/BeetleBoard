/**
 * @file esc_driver.cpp
 * @brief Implementation of ESC PWM generation for BeetleBoard
 */

#include "esc_driver.h"
#include <esp_arduino_version.h>

EscDriver::EscDriver() {}

void EscDriver::begin(uint32_t frequencyHz) {
    _frequencyHz = frequencyHz;
    _periodUs = 1000000UL / _frequencyHz;
    _resolutionBits = 14; // 14-bit resolution at 50Hz gives ~1.22us per tick
    _maxDuty = (1UL << _resolutionBits) - 1;

    for (uint8_t i = 0; i < NUM_ESC_CHANNELS; i++) {
        uint8_t pin = ESC_PINS[i];
#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
        // Modern ESP32 Core 3.x LEDC API
        ledcAttach(pin, _frequencyHz, _resolutionBits);
        ledcWrite(pin, usToDuty(PWM_DISARM_US));
#else
        // Legacy ESP32 Core 2.x LEDC API
        ledcSetup(i, _frequencyHz, _resolutionBits);
        ledcAttachPin(pin, i);
        ledcWrite(i, usToDuty(PWM_DISARM_US));
#endif
        _channelPulses[i] = PWM_DISARM_US;
    }
}

uint32_t EscDriver::usToDuty(uint16_t pulseUs) const {
    if (pulseUs > _periodUs) pulseUs = _periodUs;
    return ((uint64_t)pulseUs * _maxDuty) / _periodUs;
}

void EscDriver::setArmed(bool armed) {
    _armed = armed;
    if (!_armed) {
        emergencyStop();
    }
}

void EscDriver::emergencyStop() {
    for (uint8_t i = 0; i < NUM_ESC_CHANNELS; i++) {
        setPulseUs(i, PWM_DISARM_US);
    }
}

void EscDriver::setPulseUs(uint8_t channel, uint16_t pulseUs) {
    if (channel >= NUM_ESC_CHANNELS) return;

    // Safety clamp
    if (pulseUs < PWM_PULSE_MIN_US) pulseUs = PWM_PULSE_MIN_US;
    if (pulseUs > PWM_PULSE_MAX_US) pulseUs = PWM_PULSE_MAX_US;

    // If disarmed, force stop pulse
    if (!_armed) {
        pulseUs = PWM_DISARM_US;
    }

    _channelPulses[channel] = pulseUs;
    uint32_t duty = usToDuty(pulseUs);
    uint8_t pin = ESC_PINS[channel];

#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
    ledcWrite(pin, duty);
#else
    ledcWrite(channel, duty);
#endif
}

void EscDriver::setUnidirectional(uint8_t channel, float throttle) {
    if (throttle < 0.0f) throttle = 0.0f;
    if (throttle > 1.0f) throttle = 1.0f;

    uint16_t pulse = (uint16_t)(PWM_PULSE_MIN_US + throttle * (PWM_PULSE_MAX_US - PWM_PULSE_MIN_US));
    setPulseUs(channel, pulse);
}

void EscDriver::setBidirectional(uint8_t channel, float throttle) {
    if (throttle < -1.0f) throttle = -1.0f;
    if (throttle > 1.0f) throttle = 1.0f;

    uint16_t pulse;
    if (throttle >= 0.0f) {
        pulse = (uint16_t)(PWM_PULSE_NEUTRAL_US + throttle * (PWM_PULSE_MAX_US - PWM_PULSE_NEUTRAL_US));
    } else {
        pulse = (uint16_t)(PWM_PULSE_NEUTRAL_US + throttle * (PWM_PULSE_NEUTRAL_US - PWM_PULSE_MIN_US));
    }
    setPulseUs(channel, pulse);
}

uint16_t EscDriver::getPulseUs(uint8_t channel) const {
    if (channel >= NUM_ESC_CHANNELS) return 0;
    return _channelPulses[channel];
}

uint8_t EscDriver::getPin(uint8_t channel) const {
    if (channel >= NUM_ESC_CHANNELS) return 255;
    return ESC_PINS[channel];
}
