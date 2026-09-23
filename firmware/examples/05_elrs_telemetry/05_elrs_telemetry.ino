/**
 * @file 05_elrs_telemetry.ino
 * @brief BeetleBoard Example 5: ExpressLRS CRSF Receiver on Serial1
 * 
 * Hardware:
 * - J18 Pin 1 (XIAO_TX): GPIO 16 (Connects to ELRS Receiver RX)
 * - J18 Pin 2 (XIAO_RX): GPIO 17 (Connects to ELRS Receiver TX)
 * - J18 Pin 3: +5V
 * - J18 Pin 4: GND
 */

#define PIN_ELRS_TX 16
#define PIN_ELRS_RX 17
#define CRSF_BAUD   420000

uint16_t rcChannels[16];
uint32_t lastPacketTime = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n=== BeetleBoard Example 05: ExpressLRS CRSF Decoder ===");
  Serial.printf("Connecting Serial1 on RX=GPIO %d, TX=GPIO %d at %d baud...\n",
                PIN_ELRS_RX, PIN_ELRS_TX, CRSF_BAUD);

  Serial1.begin(CRSF_BAUD, SERIAL_8N1, PIN_ELRS_RX, PIN_ELRS_TX);
}

void loop() {
  // Simple byte stream reader for CRSF RC Channels (Type 0x16)
  static uint8_t buf[64];
  static uint8_t idx = 0;
  static uint8_t len = 0;

  while (Serial1.available()) {
    uint8_t b = Serial1.read();

    if (idx == 0) {
      if (b == 0xC8) buf[idx++] = b; // Sync byte
    } else if (idx == 1) {
      len = b;
      if (len >= 2 && len <= 62) buf[idx++] = b;
      else idx = 0;
    } else {
      buf[idx++] = b;
      if (idx == (len + 2)) {
        // Full packet arrived
        if (buf[2] == 0x16 && len >= 24) { // RC Channels packed
          const uint8_t* p = &buf[3];
          uint16_t ch0 = ((p[0]    | p[1] << 8)                       ) & 0x07FF;
          uint16_t ch1 = ((p[1]>>3 | p[2] << 5)                       ) & 0x07FF;
          uint16_t ch2 = ((p[2]>>6 | p[3] << 2 | p[4] << 10)          ) & 0x07FF;
          uint16_t ch3 = ((p[4]>>1 | p[5] << 7)                       ) & 0x07FF;
          uint16_t aux = ((p[5]>>4 | p[6] << 4)                       ) & 0x07FF;

          // Convert from 11-bit CRSF raw to microseconds
          uint16_t rollUs     = 1500 + (((ch0 - 992) * 5) / 8);
          uint16_t pitchUs    = 1500 + (((ch1 - 992) * 5) / 8);
          uint16_t throttleUs = 1500 + (((ch2 - 992) * 5) / 8);
          uint16_t yawUs      = 1500 + (((ch3 - 992) * 5) / 8);
          uint16_t aux1Us     = 1500 + (((aux - 992) * 5) / 8);

          lastPacketTime = millis();
          Serial.printf("Roll:%4dus  Pitch:%4dus  Throttle:%4dus  Yaw:%4dus  AUX1:%4dus (ARM: %s)\n",
                        rollUs, pitchUs, throttleUs, yawUs, aux1Us,
                        aux1Us > 1600 ? "ARMED" : "DISARMED");
        }
        idx = 0;
      }
    }
  }

  static uint32_t lastPrint = 0;
  if (millis() - lastPrint >= 1000) {
    lastPrint = millis();
    if (millis() - lastPacketTime > 500) {
      Serial.println("[WAIT] Waiting for ELRS CRSF packets on Serial1...");
    }
  }
}
