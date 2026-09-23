/**
 * @file 03_esc_motor_test.ino
 * @brief BeetleBoard Example 3: 6-Channel ESC PWM Generator (ESCs A-F)
 * 
 * Hardware:
 * - ESC A: GPIO 2  (J2)
 * - ESC B: GPIO 21 (J3)
 * - ESC C: GPIO 22 (J4)
 * - ESC D: GPIO 5  (J5)
 * - ESC E: GPIO 7  (J6)
 * - ESC F: GPIO 6  (J7)
 * - BUCK_EN: GPIO 1 (Powers 5V rail to ESC signal headers)
 */

#include <esp_arduino_version.h>

#define PIN_BUCK_EN 1

const uint8_t ESC_PINS[6] = {2, 21, 22, 5, 7, 6};
const char ESC_NAMES[6]   = {'A', 'B', 'C', 'D', 'E', 'F'};

const uint32_t PWM_FREQ = 50;       // 50Hz standard RC PWM (20ms period)
const uint8_t  PWM_RES  = 14;       // 14-bit resolution (0..16383)
const uint32_t MAX_DUTY = 16383;
const uint32_t PERIOD_US = 20000;

uint32_t usToDuty(uint16_t pulseUs) {
  return ((uint64_t)pulseUs * MAX_DUTY) / PERIOD_US;
}

void setEscPulse(uint8_t index, uint16_t pulseUs) {
  uint32_t duty = usToDuty(pulseUs);
  uint8_t pin = ESC_PINS[index];
#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
  ledcWrite(pin, duty);
#else
  ledcWrite(index, duty);
#endif
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n=== BeetleBoard Example 03: ESC 6-Channel Motor Test ===");

  // Ensure 5V rail is on for ESC signal headers
  pinMode(PIN_BUCK_EN, OUTPUT);
  digitalWrite(PIN_BUCK_EN, HIGH);

  // Initialize all 6 ESC PWM channels
  for (uint8_t i = 0; i < 6; i++) {
    uint8_t pin = ESC_PINS[i];
#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
    ledcAttach(pin, PWM_FREQ, PWM_RES);
#else
    ledcSetup(i, PWM_FREQ, PWM_RES);
    ledcAttachPin(pin, i);
#endif
    setEscPulse(i, 1000); // Disarmed / safe low pulse (1000us)
  }

  Serial.println("[OK] ESCs A-F initialized at 50Hz (1000us idle).");
  Serial.println("     Type 't' in Serial Monitor to run a safe test ramp on ESC A.");
}

void loop() {
  if (Serial.available()) {
    char c = Serial.read();
    if (c == 't') {
      Serial.println("\n[TEST] Running gentle sweep on ESC A (GPIO 2)...");
      // Ramp from 1000us to 1200us
      for (uint16_t p = 1000; p <= 1200; p += 5) {
        setEscPulse(0, p);
        delay(25);
      }
      delay(500);
      // Ramp back down to 1000us
      for (uint16_t p = 1200; p >= 1000; p -= 5) {
        setEscPulse(0, p);
        delay(25);
      }
      Serial.println("[TEST] Done. ESC A back to 1000us.");
    }
  }
}
