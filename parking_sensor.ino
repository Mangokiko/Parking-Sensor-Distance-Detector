/*
  ============================================================
  Parking Sensor / Distance Alert
  CS 220: Computer Architecture & Embedded Systems
  Capstone Project
  ============================================================
  COMPONENTS:
    - Arduino Uno
    - HC-SR04 Ultrasonic Sensor
    - LCD 16x2 Display
    - Potentiometer (LCD contrast)
    - Green, Yellow, Red LEDs
    - Piezo Buzzer
    - 470Ω resistors (LEDs), 220Ω resistor (LCD backlight)

  PIN MAPPING:
    Ultrasonic → TRIG=7, ECHO=6
    LCD        → RS=12, EN=11, D4=5, D5=4, D6=3, D7=2
    LEDs       → Green=8, Yellow=9, Red=10
    Buzzer     → Pin 13
  ============================================================
*/

#include <LiquidCrystal.h>

// ---------- LCD Setup ----------
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// ---------- Ultrasonic Sensor Pins ----------
const int TRIG_PIN = 7;
const int ECHO_PIN = 6;

// ---------- LED Pins ----------
const int GREEN_LED  = 8;
const int YELLOW_LED = 9;
const int RED_LED    = 10;

// ---------- Buzzer Pin ----------
const int BUZZER_PIN = 13;

// ---------- Distance Thresholds (cm) ----------
const int SAFE_DIST    = 40;   // Green  → far away, all clear
const int CAUTION_DIST = 20;   // Yellow → getting close
const int DANGER_DIST  = 10;   // Red    → very close, beep fast

// ---------- Timing for beeping ----------
unsigned long lastBeepTime = 0;
int beepInterval = 500;        // ms between beeps (gets shorter as closer)

// ============================================================
// SETUP
// ============================================================
void setup() {
  // LED pins as OUTPUT
  pinMode(GREEN_LED,  OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(RED_LED,    OUTPUT);

  // Buzzer as OUTPUT
  pinMode(BUZZER_PIN, OUTPUT);

  // Ultrasonic pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Start LCD
  lcd.begin(16, 2);

  // Welcome message
  lcd.setCursor(0, 0);
  lcd.print(" PARKING SENSOR ");
  lcd.setCursor(0, 1);
  lcd.print("  Initializing  ");
  delay(2000);
  lcd.clear();
}

// ============================================================
// MAIN LOOP
// ============================================================
void loop() {
  // Get distance reading
  long distance = getDistance();

  // Update LCD display
  updateDisplay(distance);

  // Update LEDs based on distance
  updateLEDs(distance);

  // Update buzzer based on distance
  updateBuzzer(distance);

  delay(100);  // Small delay between readings for stability
}

// ============================================================
// FUNCTIONS
// ============================================================

// --- Measure distance using HC-SR04 ---
long getDistance() {
  // Send a 10 microsecond pulse to TRIG
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Read the ECHO pulse duration
  long duration = pulseIn(ECHO_PIN, HIGH);

  // Convert duration to distance in cm
  // Speed of sound = 343 m/s = 0.0343 cm/µs
  // Divide by 2 because pulse travels to object AND back
  long distance = duration * 0.0343 / 2;

  // Cap at 200cm for display purposes
  if (distance > 200) distance = 200;
  if (distance < 0)   distance = 0;

  return distance;
}

// --- Update the LCD with current distance and status ---
void updateDisplay(long distance) {
  // Row 1: Distance reading
  lcd.setCursor(0, 0);
  lcd.print("Dist: ");
  lcd.print(distance);
  lcd.print(" cm    ");   // Extra spaces clear old characters

  // Row 2: Status message
  lcd.setCursor(0, 1);
  if (distance > SAFE_DIST) {
    lcd.print("  ALL CLEAR!    ");
  } else if (distance > CAUTION_DIST) {
    lcd.print("  SLOW DOWN...  ");
  } else if (distance > DANGER_DIST) {
    lcd.print("  CAUTION!!!    ");
  } else {
    lcd.print("   ** STOP! **  ");
  }
}

// --- Update LEDs based on distance ---
void updateLEDs(long distance) {
  if (distance > SAFE_DIST) {
    // Far away — green only
    digitalWrite(GREEN_LED,  HIGH);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(RED_LED,    LOW);
  } else if (distance > CAUTION_DIST) {
    // Getting close — yellow only
    digitalWrite(GREEN_LED,  LOW);
    digitalWrite(YELLOW_LED, HIGH);
    digitalWrite(RED_LED,    LOW);
  } else if (distance > DANGER_DIST) {
    // Close — yellow + red
    digitalWrite(GREEN_LED,  LOW);
    digitalWrite(YELLOW_LED, HIGH);
    digitalWrite(RED_LED,    HIGH);
  } else {
    // Very close — red only, flashing
    digitalWrite(GREEN_LED,  LOW);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(RED_LED,    HIGH);
  }
}

// --- Update buzzer beep rate based on distance ---
void updateBuzzer(long distance) {
  // No beep when safe distance
  if (distance > SAFE_DIST) {
    noTone(BUZZER_PIN);
    return;
  }

  // Beep interval gets shorter as object gets closer
  if (distance > CAUTION_DIST) {
    beepInterval = 600;        // Slow beep
  } else if (distance > DANGER_DIST) {
    beepInterval = 250;        // Fast beep
  } else {
    beepInterval = 80;         // Very fast beep (almost solid tone)
  }

  // Non-blocking beep using millis()
  unsigned long currentTime = millis();
  if (currentTime - lastBeepTime >= beepInterval) {
    lastBeepTime = currentTime;
    tone(BUZZER_PIN, 1000, 50);  // 1000Hz tone, 50ms duration
  }
}
