/**
 * @file 04_imu_read_spi.ino
 * @brief BeetleBoard Example 4: LSM6DSV320X 6-Axis IMU via SPI
 * 
 * Hardware:
 * - CS:   GPIO 23 (PIN_IMU_CS)
 * - SCK:  GPIO 19 (PIN_IMU_SCK)
 * - MISO: GPIO 20 (PIN_IMU_MISO)
 * - MOSI: GPIO 18 (PIN_IMU_MOSI)
 */

#include <SPI.h>

#define PIN_IMU_CS   23
#define PIN_IMU_SCK  19
#define PIN_IMU_MISO 20
#define PIN_IMU_MOSI 18

SPISettings spiSettings(4000000, MSBFIRST, SPI_MODE0);

uint8_t readReg(uint8_t reg) {
  SPI.beginTransaction(spiSettings);
  digitalWrite(PIN_IMU_CS, LOW);
  SPI.transfer(reg | 0x80); // Bit 7 = 1 for read
  uint8_t val = SPI.transfer(0x00);
  digitalWrite(PIN_IMU_CS, HIGH);
  SPI.endTransaction();
  return val;
}

void writeReg(uint8_t reg, uint8_t val) {
  SPI.beginTransaction(spiSettings);
  digitalWrite(PIN_IMU_CS, LOW);
  SPI.transfer(reg & 0x7F); // Bit 7 = 0 for write
  SPI.transfer(val);
  digitalWrite(PIN_IMU_CS, HIGH);
  SPI.endTransaction();
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n=== BeetleBoard Example 04: LSM6DSV320X IMU (SPI) ===");

  pinMode(PIN_IMU_CS, OUTPUT);
  digitalWrite(PIN_IMU_CS, HIGH);

  // Initialize SPI with BeetleBoard pins
  SPI.begin(PIN_IMU_SCK, PIN_IMU_MISO, PIN_IMU_MOSI, PIN_IMU_CS);
  delay(50);

  // Check WHO_AM_I (reg 0x0F) -> expected 0x73
  uint8_t id = readReg(0x0F);
  Serial.printf("WHO_AM_I read: 0x%02X (Expected: 0x73)\n", id);

  if (id != 0x73) {
    Serial.println("[ERROR] LSM6DSV320X IMU not detected! Check soldering and connections.");
    while (1) delay(1000);
  }

  // Soft reset (CTRL3, 0x12)
  writeReg(0x12, 0x01);
  delay(20);

  // Set BDU=1, IF_INC=1
  writeReg(0x12, 0x44);

  // Enable Accel: 120Hz ODR, ±16g (CTRL1_XL, 0x10)
  writeReg(0x10, 0x36);

  // Enable Gyro: 120Hz ODR, ±2000 dps (CTRL2_G, 0x11)
  writeReg(0x11, 0x46);

  Serial.println("[OK] LSM6DSV320X configured at 120Hz, ±16G, ±2000 dps.\n");
}

void loop() {
  uint8_t buf[12];

  SPI.beginTransaction(spiSettings);
  digitalWrite(PIN_IMU_CS, LOW);
  SPI.transfer(0x22 | 0x80); // Start burst read at OUTX_L_G (0x22)
  for (int i = 0; i < 12; i++) {
    buf[i] = SPI.transfer(0x00);
  }
  digitalWrite(PIN_IMU_CS, HIGH);
  SPI.endTransaction();

  int16_t gx = (int16_t)((buf[1] << 8) | buf[0]);
  int16_t gy = (int16_t)((buf[3] << 8) | buf[2]);
  int16_t gz = (int16_t)((buf[5] << 8) | buf[4]);

  int16_t ax = (int16_t)((buf[7] << 8) | buf[6]);
  int16_t ay = (int16_t)((buf[9] << 8) | buf[8]);
  int16_t az = (int16_t)((buf[11] << 8) | buf[10]);

  // Scale: Accel ±16g -> ~0.000488 g/LSB; Gyro ±2000 dps -> ~0.070 dps/LSB
  float ax_g = (float)ax * 0.000488f;
  float ay_g = (float)ay * 0.000488f;
  float az_g = (float)az * 0.000488f;

  float gx_dps = (float)gx * 0.070f;
  float gy_dps = (float)gy * 0.070f;
  float gz_dps = (float)gz * 0.070f;

  Serial.printf("Accel [g]: X:%+5.2f Y:%+5.2f Z:%+5.2f | Gyro [dps]: X:%+6.1f Y:%+6.1f Z:%+6.1f\n",
                ax_g, ay_g, az_g, gx_dps, gy_dps, gz_dps);

  delay(100);
}
