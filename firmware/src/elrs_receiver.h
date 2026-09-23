/**
 * @file elrs_receiver.h
 * @brief ExpressLRS / CRSF Serial Receiver Driver for BeetleBoard
 */

#pragma once

#include <Arduino.h>
#include "beetleboard_pins.h"

#define CRSF_MAX_CHANNELS           16
#define CRSF_SYNC_BYTE              0xC8
#define CRSF_FRAMETYPE_RC_CHANNELS  0x16
#define CRSF_FRAMETYPE_LINK_STATS   0x14
#define CRSF_TIMEOUT_MS             250

struct CrsfLinkStats {
    uint8_t uplinkRssi1;    // RSSI antenna 1 (-dBm)
    uint8_t uplinkRssi2;    // RSSI antenna 2 (-dBm)
    uint8_t uplinkLinkQuality; // 0-100%
    int8_t  uplinkSnr;      // SNR (dB)
    uint8_t activeAntenna;
    uint8_t rfMode;
    uint8_t uplinkTxPower;  // mW
};

class ElrsReceiver {
public:
    ElrsReceiver();

    /**
     * @brief Initialize Hardware Serial for ELRS CRSF protocol.
     * @param baudrate CRSF standard is 420000.
     */
    void begin(uint32_t baudrate = ELRS_CRSF_BAUDRATE);

    /**
     * @brief Periodically parse incoming serial bytes.
     */
    void update();

    /**
     * @brief Check whether valid ELRS RC packets are currently being received.
     */
    bool isConnected() const;

    /**
     * @brief Get channel value mapped to RC microseconds (1000us to 2000us).
     * @param channel 0-indexed (0=Roll, 1=Pitch, 2=Throttle, 3=Yaw, 4=AUX1/Arm, etc.)
     */
    uint16_t getChannelUs(uint8_t channel) const;

    /**
     * @brief Get normalized channel value:
     *        - Centered channels (Roll/Pitch/Yaw): -1.0f to +1.0f
     *        - Unidirectional (Throttle/Switches): 0.0f to 1.0f
     */
    float getChannelBidirectional(uint8_t channel) const;
    float getChannelUnidirectional(uint8_t channel) const;

    /**
     * @brief Check arm switch (typically Channel 4 / AUX1 in ELRS).
     */
    bool isArmSwitchActive(uint8_t auxChannel = 4) const;

    /**
     * @brief Get latest link statistics.
     */
    const CrsfLinkStats& getLinkStats() const { return _linkStats; }

private:
    void processByte(uint8_t b);
    void handleFrame();
    void unpackChannels(const uint8_t* payload);
    uint8_t calculateCrc(const uint8_t* data, uint8_t length);

    uint16_t _channels[CRSF_MAX_CHANNELS];
    CrsfLinkStats _linkStats = {};

    uint8_t _buffer[64];
    uint8_t _bufIndex = 0;
    uint8_t _expectedLen = 0;
    uint32_t _lastPacketTime = 0;
};
