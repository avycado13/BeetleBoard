/**
 * @file 02_battery_monitor.ino
 * @brief BeetleBoard Example 2: Battery Voltage Sensing & Software UVLO
 * 
 * Hardware:
 * - Voltage divider: R10 (120kΩ) and R9 (33.2kΩ) connected to VBAT_SNS (GPIO 0)
 * - Buck Enable: BUCK_EN (GPIO 1) connected to TPS563200 EN pin
 */

#define PIN_VBAT_SNS    0
#define PIN_BUCK_EN     1
#define PIN_LED_USER    15

// Voltage divider ratio calculation:
// V_pin = V_bat * (33.2k / (120k + 33.2k)) = V_bat * 0.21671
// V_bat = V_pin * (153.2k / 33.2k) = V_pin * 4.61446
const float VBAT_MULTIPLIER = (120000.0f + 33200.0f) / 33200.0f;

// Cutoff threshold per cell for LiPo batteries (V)
const float CELL_CUTOFF_V = 3.30f;
const float CELL_WARN_V   = 3.50f;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n=== BeetleBoard Example 02: Battery Voltage & UVLO ===");

  pinMode(PIN_BUCK_EN, OUTPUT);
  digitalWrite(PIN_BUCK_EN, HIGH); // Default: enable 5V rail

  pinMode(PIN_LED_USER, OUTPUT);
  pinMode(PIN_VBAT_SNS, INPUT);
  analogSetPinAttenuation(PIN_VBAT_SNS, ADC_11db);
}

void loop() {
  // Read calibrated millivolts from ESP32 ADC
  uint32_t pinMv = analogReadMilliVolts(PIN_VBAT_SNS);
  float vbat = ((float)pinMv / 1000.0f) * VBAT_MULTIPLIER;

  // Determine cell count
  uint8_t cells = 0;
  if (vbat > 13.0f) cells = 4;
  else if (vbat > 8.8f) cells = 3; // Common for beetleweights (3S LiPo)
  else if (vbat > 5.5f) cells = 2;

  float cellV = (cells > 0) ? (vbat / cells) : 0.0f;

  Serial.printf("Vbat: %5.2f V | Pin: %4u mV | Detected: %dS | Cell: %4.2f V/cell | ",
                vbat, pinMv, cells, cellV);

  // Check Undervoltage Lockout
  if (cells > 0 && cellV < CELL_CUTOFF_V) {
    digitalWrite(PIN_BUCK_EN, LOW); // Cut off 5V rail to save battery!
    digitalWrite(PIN_LED_USER, LOW); // LED ON
    Serial.println("[UVLO CUTOFF] 5V Rail DISABLED to protect battery!");
  } else if (cells > 0 && cellV < CELL_WARN_V) {
    digitalWrite(PIN_BUCK_EN, HIGH);
    Serial.println("[WARNING] Battery low!");
  } else {
    digitalWrite(PIN_BUCK_EN, HIGH);
    Serial.println("[OK]");
  }

  delay(500);
}
