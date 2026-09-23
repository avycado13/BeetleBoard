/**
 * @file elrs_receiver.cpp
 * @brief Implementation of ExpressLRS / CRSF Serial Protocol Decoder
 */

#include "elrs_receiver.h"

// CRSF CRC8 Lookup Table (Polynomial 0xD5)
static const uint8_t crsfCrcTable[256] = {
    0x00, 0xD5, 0x7F, 0xAA, 0xFE, 0x2B, 0x81, 0x54, 0x29, 0xFC, 0x56, 0x83, 0xD7, 0x02, 0xA8, 0x7D,
    0x52, 0x87, 0x2D, 0xF8, 0xAC, 0x79, 0xD3, 0x06, 0x7B, 0xAE, 0x04, 0xD1, 0x85, 0x50, 0xFA, 0x2F,
    0xA4, 0x71, 0xDB, 0x0E, 0x5A, 0x8F, 0x25, 0xF0, 0x8D, 0x58, 0xF2, 0x27, 0x73, 0xA6, 0x0C, 0xD9,
    0xF6, 0x23, 0x89, 0x5C, 0x08, 0xDD, 0x77, 0xA2, 0xDF, 0x0A, 0xA0, 0x75, 0x21, 0xF4, 0x5E, 0x8B,
    0x9D, 0x48, 0xE2, 0x37, 0x63, 0xB6, 0x1C, 0xC9, 0xB4, 0x61, 0xCB, 0x1E, 0x4A, 0x9F, 0x35, 0xE0,
    0xCF, 0x1A, 0xB0, 0x65, 0x31, 0xE4, 0x4E, 0x9B, 0xE6, 0x33, 0x99, 0x4C, 0x18, 0xCD, 0x67, 0xB2,
    0x39, 0xEC, 0x46, 0x93, 0xC7, 0x12, 0xB8, 0x6D, 0x10, 0xC5, 0x6F, 0xBA, 0xEE, 0x3B, 0x91, 0x44,
    0x6B, 0xBE, 0x14, 0xC1, 0x95, 0x40, 0xEA, 0x3F, 0x42, 0x97, 0x3D, 0xE8, 0xBC, 0x69, 0xC3, 0x16,
    0xEF, 0x3A, 0x90, 0x45, 0x11, 0xC4, 0x6E, 0xBB, 0xC6, 0x13, 0xB9, 0x6C, 0x38, 0xED, 0x47, 0x92,
    0xBD, 0x68, 0xC2, 0x17, 0x43, 0x96, 0x3C, 0xE9, 0x94, 0x41, 0xEB, 0x3E, 0x6A, 0xBF, 0x15, 0xC0,
    0x4B, 0x9E, 0x34, 0xE1, 0xB5, 0x60, 0xCA, 0x1F, 0x62, 0xB7, 0x1D, 0xC8, 0x9C, 0x49, 0xE3, 0x36,
    0x19, 0xCC, 0x66, 0xB3, 0xE7, 0x32, 0x98, 0x4D, 0x30, 0xE5, 0x4F, 0x9A, 0xCE, 0x1B, 0xB1, 0x64,
    0x72, 0xA7, 0x0D, 0xD8, 0x8C, 0x59, 0xF3, 0x26, 0x5B, 0x8E, 0x24, 0xF1, 0xA5, 0x70, 0xDA, 0x0F,
    0x20, 0xF5, 0x5F, 0x8A, 0xDE, 0x0B, 0xA1, 0x74, 0x09, 0xDC, 0x76, 0xA3, 0xF7, 0x22, 0x88, 0x5D,
    0xD6, 0x03, 0xA9, 0x7C, 0x28, 0xFD, 0x57, 0x82, 0xFF, 0x2A, 0x80, 0x55, 0x01, 0xD4, 0x7E, 0xAB,
    0x84, 0x51, 0xFB, 0x2E, 0x7A, 0xAF, 0x05, 0xD0, 0xAD, 0x78, 0xD2, 0x07, 0x53, 0x86, 0x2C, 0xF9
};

ElrsReceiver::ElrsReceiver() {
    for (uint8_t i = 0; i < CRSF_MAX_CHANNELS; i++) {
        _channels[i] = PWM_PULSE_NEUTRAL_US;
    }
    _channels[2] = PWM_PULSE_MIN_US; // Throttle channel default to min
}

void ElrsReceiver::begin(uint32_t baudrate) {
    // Configure Serial1 on pins: RX = GPIO 17, TX = GPIO 16
    ELRS_SERIAL_PORT.begin(baudrate, SERIAL_8N1, PIN_ELRS_RX, PIN_ELRS_TX);
}

bool ElrsReceiver::isConnected() const {
    return (_lastPacketTime > 0) && ((millis() - _lastPacketTime) < CRSF_TIMEOUT_MS);
}

void ElrsReceiver::update() {
    while (ELRS_SERIAL_PORT.available()) {
        uint8_t b = ELRS_SERIAL_PORT.read();
        processByte(b);
    }
}

void ElrsReceiver::processByte(uint8_t b) {
    if (_bufIndex == 0) {
        if (b == CRSF_SYNC_BYTE) {
            _buffer[_bufIndex++] = b;
        }
        return;
    }

    if (_bufIndex == 1) {
        _expectedLen = b;
        if (_expectedLen < 2 || _expectedLen > 62) {
            _bufIndex = 0;
            return;
        }
        _buffer[_bufIndex++] = b;
        return;
    }

    _buffer[_bufIndex++] = b;

    // When entire frame (Sync + Len + Length bytes) has arrived
    if (_bufIndex == (_expectedLen + 2)) {
        handleFrame();
        _bufIndex = 0;
    }
}

uint8_t ElrsReceiver::calculateCrc(const uint8_t* data, uint8_t length) {
    uint8_t crc = 0;
    for (uint8_t i = 0; i < length; i++) {
        crc = crsfCrcTable[crc ^ data[i]];
    }
    return crc;
}

void ElrsReceiver::handleFrame() {
    // Length covers: Type + Payload + CRC
    uint8_t frameType = _buffer[2];
    uint8_t payloadLen = _expectedLen - 2;
    uint8_t receivedCrc = _buffer[_bufIndex - 1];

    // Verify CRC (over Type + Payload)
    uint8_t computedCrc = calculateCrc(&_buffer[2], _expectedLen - 1);
    if (computedCrc != receivedCrc) {
        return; // CRC error
    }

    _lastPacketTime = millis();

    if (frameType == CRSF_FRAMETYPE_RC_CHANNELS && payloadLen >= 22) {
        unpackChannels(&_buffer[3]);
    } else if (frameType == CRSF_FRAMETYPE_LINK_STATS && payloadLen >= 10) {
        const uint8_t* p = &_buffer[3];
        _linkStats.uplinkRssi1 = p[0];
        _linkStats.uplinkRssi2 = p[1];
        _linkStats.uplinkLinkQuality = p[2];
        _linkStats.uplinkSnr = (int8_t)p[3];
        _linkStats.activeAntenna = p[4];
        _linkStats.rfMode = p[5];
        _linkStats.uplinkTxPower = p[6];
    }
}

void ElrsReceiver::unpackChannels(const uint8_t* p) {
    uint16_t raw[16];
    raw[0]  = ((p[0]    | p[1] << 8)                       ) & 0x07FF;
    raw[1]  = ((p[1]>>3 | p[2] << 5)                       ) & 0x07FF;
    raw[2]  = ((p[2]>>6 | p[3] << 2 | p[4] << 10)          ) & 0x07FF;
    raw[3]  = ((p[4]>>1 | p[5] << 7)                       ) & 0x07FF;
    raw[4]  = ((p[5]>>4 | p[6] << 4)                       ) & 0x07FF;
    raw[5]  = ((p[6]>>7 | p[7] << 1 | p[8] << 9)           ) & 0x07FF;
    raw[6]  = ((p[8]>>2 | p[9] << 6)                       ) & 0x07FF;
    raw[7]  = ((p[9]>>5 | p[10] << 3)                      ) & 0x07FF;
    raw[8]  = ((p[11]   | p[12] << 8)                      ) & 0x07FF;
    raw[9]  = ((p[12]>>3| p[13] << 5)                      ) & 0x07FF;
    raw[10] = ((p[13]>>6| p[14] << 2 | p[15] << 10)        ) & 0x07FF;
    raw[11] = ((p[15]>>1| p[16] << 7)                      ) & 0x07FF;
    raw[12] = ((p[16]>>4| p[17] << 4)                      ) & 0x07FF;
    raw[13] = ((p[17]>>7| p[18] << 1 | p[19] << 9)         ) & 0x07FF;
    raw[14] = ((p[19]>>2| p[20] << 6)                      ) & 0x07FF;
    raw[15] = ((p[20]>>5| p[21] << 3)                      ) & 0x07FF;

    // Convert raw 11-bit counts (172..1811) to microseconds (1000..2000 us)
    for (uint8_t i = 0; i < CRSF_MAX_CHANNELS; i++) {
        int32_t val = (int32_t)raw[i];
        int32_t us = 1500 + (((val - 992) * 5) / 8);
        if (us < PWM_PULSE_MIN_US) us = PWM_PULSE_MIN_US;
        if (us > PWM_PULSE_MAX_US) us = PWM_PULSE_MAX_US;
        _channels[i] = (uint16_t)us;
    }
}

uint16_t ElrsReceiver::getChannelUs(uint8_t channel) const {
    if (channel >= CRSF_MAX_CHANNELS) return PWM_PULSE_NEUTRAL_US;
    return _channels[channel];
}

float ElrsReceiver::getChannelBidirectional(uint8_t channel) const {
    uint16_t us = getChannelUs(channel);
    return ((float)us - 1500.0f) / 500.0f; // -1.0 to +1.0
}

float ElrsReceiver::getChannelUnidirectional(uint8_t channel) const {
    uint16_t us = getChannelUs(channel);
    return ((float)us - 1000.0f) / 1000.0f; // 0.0 to 1.0
}

bool ElrsReceiver::isArmSwitchActive(uint8_t auxChannel) const {
    // Channel 4 is AUX1 (commonly the Arm switch on ELRS transmitters)
    return getChannelUs(auxChannel) > 1600;
}
