/**
 * @file 01_blink_and_power.ino
 * @brief BeetleBoard Example 1: 5V Buck Rail Control & Status LED
 * 
 * Target: Seeed Studio XIAO ESP32-C6 on BeetleBoard
 * 
 * Demonstrates:
 * - Enabling the onboard TPS563200 3A 5V Buck Regulator via BUCK_EN (GPIO 1)
 * - Toggling the onboard active-low user LED (GPIO 15)
 */

#define PIN_BUCK_EN     1   // Controls TPS563200 EN pin (Active-HIGH)
#define PIN_LED_USER    15  // XIAO ESP32-C6 onboard LED (Active-LOW)

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n=== BeetleBoard Example 01: Blink & Power Rail ===");

  // Configure BUCK_EN as output and drive HIGH to power the 5V rail
  pinMode(PIN_BUCK_EN, OUTPUT);
  digitalWrite(PIN_BUCK_EN, HIGH);
  Serial.println("[OK] TPS563200 5V Buck Regulator ENABLED (Pin 1 -> HIGH)");
  Serial.println("     The +5V rail powering ESC headers and ELRS is now active.");

  // Configure user LED
  pinMode(PIN_LED_USER, OUTPUT);
}

void loop() {
  // Blink heartbeat
  digitalWrite(PIN_LED_USER, LOW);  // LED ON (active-low)
  delay(500);
  digitalWrite(PIN_LED_USER, HIGH); // LED OFF
  delay(500);
}
