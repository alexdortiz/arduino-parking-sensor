/*
  Parking Sensor 
  
    HC-SR04: TRIG=7, ECHO=6
    LCD1602: RS=12, EN=11, D4=5, D5=4, D6=3, D7=2
    LEDs:    Green=8, Yellow=9, Red=10
    Buzzer:  13

*/

#include <LiquidCrystal.h>

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// ---- Pins ----
enum Pins : uint8_t {
  PIN_TRIG   = 7,
  PIN_ECHO   = 6,
  PIN_GREEN  = 8,
  PIN_YELLOW = 9,
  PIN_RED    = 10,
  PIN_BUZZER = 13
};

// ---- Zones, ordered nearest to farthest ----
enum Zone : uint8_t { ZONE_STOP, ZONE_CAUTION, ZONE_SLOW, ZONE_CLEAR, ZONE_COUNT };

// One row per zone: everything the outputs need to know.
struct ZoneConfig {
  int         minDistance;   // zone applies when distance > minDistance
  const char *message;       // 16-char LCD status line
  bool        green, yellow, red;
  int         beepMs;        // ms between beeps; 0 = silent
};

// Thresholds match v1: 40 / 20 / 10 cm
const ZoneConfig ZONES[ZONE_COUNT] = {
  //                min   message              G      Y      R      beep
  /* STOP    */ { -1, "   ** STOP! **  ", false, false, true,   80 },
  /* CAUTION */ { 10, "  CAUTION!!!    ", false, true,  true,  250 },
  /* SLOW    */ { 20, "  SLOW DOWN...  ", false, true,  false, 600 },
  /* CLEAR   */ { 40, "  ALL CLEAR!    ", true,  false, false,   0 }
};

const long          MAX_RANGE_CM  = 200;
const unsigned long ECHO_TIMEOUT  = 25000UL; // µs; ~4m round trip, prevents pulseIn() hanging
const unsigned long LOOP_PERIOD   = 100;     // ms between sensor reads

unsigned long lastReadTime = 0;
unsigned long lastBeepTime = 0;
int currentBeepMs = 0;   // beep interval for the current zone; 0 = silent

void setup() {
  pinMode(PIN_TRIG,   OUTPUT);
  pinMode(PIN_ECHO,   INPUT);
  pinMode(PIN_GREEN,  OUTPUT);
  pinMode(PIN_YELLOW, OUTPUT);
  pinMode(PIN_RED,    OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);

  lcd.begin(16, 2);
  lcd.print(" PARKING SENSOR ");
  lcd.setCursor(0, 1);
  lcd.print("  Initializing  ");
  delay(2000);
  lcd.clear();
}

void loop() {
  unsigned long now = millis();

  // Read + render on a fixed period instead of a blocking delay()
  if (now - lastReadTime >= LOOP_PERIOD) {
    lastReadTime = now;

    long distance = readDistanceCm();
    Zone zone     = classify(distance);

    renderLCD(distance, ZONES[zone].message);
    renderLEDs(ZONES[zone]);
    currentBeepMs = ZONES[zone].beepMs;
  }

  // Buzzer runs every pass so beat timing stays accurate
  serviceBuzzer(now);
}

// ---- Sensor ----
long readDistanceCm() {
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);

  // Timeout so a missed echo returns 0 instead of stalling ~1 second
  unsigned long duration = pulseIn(PIN_ECHO, HIGH, ECHO_TIMEOUT);
  if (duration == 0) return MAX_RANGE_CM;  // no echo -> treat as nothing in range

  long cm = (long)(duration * 0.0343 / 2.0);
  return constrain(cm, 0, MAX_RANGE_CM);
}

// ---- Classification: the ONLY place thresholds are checked ----
Zone classify(long distance) {
  // Walk from farthest zone down; first match wins
  for (int z = ZONE_COUNT - 1; z > 0; z--) {
    if (distance > ZONES[z].minDistance) return (Zone)z;
  }
  return ZONE_STOP;
}

// ---- Outputs ----
void renderLCD(long distance, const char *status) {
  lcd.setCursor(0, 0);
  lcd.print("Dist: ");
  lcd.print(distance);
  lcd.print(" cm    ");      // padding overwrites stale digits
  lcd.setCursor(0, 1);
  lcd.print(status);
}

void renderLEDs(const ZoneConfig &z) {
  digitalWrite(PIN_GREEN,  z.green  ? HIGH : LOW);
  digitalWrite(PIN_YELLOW, z.yellow ? HIGH : LOW);
  digitalWrite(PIN_RED,    z.red    ? HIGH : LOW);
}

// serviceBuzzer reads currentBeepMs, which is refreshed each time
// the zone is re-classified in loop().

void serviceBuzzer(unsigned long now) {
  if (currentBeepMs == 0) {
    noTone(PIN_BUZZER);
    return;
  }
  if (now - lastBeepTime >= (unsigned long)currentBeepMs) {
    lastBeepTime = now;
    tone(PIN_BUZZER, 1000, 50);   // 1 kHz chirp, 50 ms
  }
}
