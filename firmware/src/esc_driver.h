/**
 * @file esc_driver.h
 * @brief Multi-channel ESC and Servo PWM driver for BeetleBoard (ESCs A-F)
 */

#pragma once

#include <Arduino.h>
#include "beetleboard_pins.h"

enum class EscChannel : uint8_t {
    ESC_A = 0,
    ESC_B = 1,
    ESC_C = 2,
    ESC_D = 3,
    ESC_E = 4,
    ESC_F = 5
};

class EscDriver {
public:
    EscDriver();

    /**
     * @brief Initialize LEDC PWM timers and attach pins for all 6 ESC channels.
     * @param frequencyHz PWM frequency (typically 50Hz for standard RC / servos, up to 400Hz).
     */
    void begin(uint32_t frequencyHz = PWM_FREQUENCY_HZ);

    /**
     * @brief Arm or disarm ESCs. When disarmed, pulses are set to safe stop.
     */
    void setArmed(bool armed);

    /**
     * @brief Check whether outputs are currently armed.
     */
    bool isArmed() const { return _armed; }

    /**
     * @brief Immediately cut all motor outputs to stop pulse width.
     */
    void emergencyStop();

    /**
     * @brief Set pulse width directly in microseconds (1000 - 2000 us).
     * @param channel Index 0 to 5 (or EscChannel enum).
     * @param pulseUs Pulse duration in microseconds.
     */
    void setPulseUs(uint8_t channel, uint16_t pulseUs);
    void setPulseUs(EscChannel channel, uint16_t pulseUs) {
        setPulseUs(static_cast<uint8_t>(channel), pulseUs);
    }

    /**
     * @brief Set throttle normalized from 0.0 (stop) to 1.0 (full speed).
     */
    void setUnidirectional(uint8_t channel, float throttle);

    /**
     * @brief Set bidirectional throttle from -1.0 (reverse) to 0.0 (stop) to +1.0 (forward).
     */
    void setBidirectional(uint8_t channel, float throttle);

    /**
     * @brief Read back current commanded pulse width in microseconds.
     */
    uint16_t getPulseUs(uint8_t channel) const;

    /**
     * @brief Return the GPIO pin number for a given ESC channel.
     */
    uint8_t getPin(uint8_t channel) const;

private:
    uint32_t usToDuty(uint16_t pulseUs) const;

    bool _armed = false;
    uint32_t _frequencyHz = 50;
    uint8_t _resolutionBits = 14;
    uint32_t _periodUs = 20000;
    uint32_t _maxDuty = 16383;

    uint16_t _channelPulses[NUM_ESC_CHANNELS] = {
        PWM_PULSE_MIN_US, PWM_PULSE_MIN_US, PWM_PULSE_MIN_US,
        PWM_PULSE_MIN_US, PWM_PULSE_MIN_US, PWM_PULSE_MIN_US
    };
};
