#include <LiquidCrystal.h>
#include <Servo.h>

// LCD: rs=5, en=6, d4=7, d5=8, d6=9, d7=10
LiquidCrystal lcd(5, 6, 7, 8, 9, 10);
const char StrH[] = "Hello!";

// TIME / измерения
volatile unsigned long lastPulseMs = 0;   // время последнего импульса (мс)
volatile unsigned long deltaMs      = 0;  // интервал между импульсами (мс)
volatile unsigned long revCount     = 0;  // число оборотов (для дистанции)

float f = 0.0f;       // Гц
float V = 0.0f;       // м/с
float L = 0.0f;       // м (длина окружности)
const float D = 0.67; // м
const float pi = 3.1415926f;

float distance_m = 0.0f;
float distance_km = 0.0f;

// --- Magnet detect ISR ---
void magnet_detect() {
  unsigned long now = millis();
  // антидребезг ~10 мс
  if (now - lastPulseMs >= 10) {
    deltaMs = now - lastPulseMs;
    lastPulseMs = now;
    revCount++; // суммируем обороты, а в loop() пересчитаем в метры
  }
}

// LED
int val = 0;
const int RES = A0; // фоторезистор на A0
void blinking() {
  digitalWrite(4, HIGH);
  digitalWrite(3, HIGH);
  delay(500);
  digitalWrite(4, LOW);
  digitalWrite(3, LOW);
  delay(100);
}

// SERVO
Servo servo;
int angle = 0;

void setup() {
  Serial.begin(9600);

  pinMode(2, INPUT_PULLUP); // геркон на D2 → GND
  attachInterrupt(digitalPinToInterrupt(2), magnet_detect, FALLING);

  pinMode(4, OUTPUT); // BACK LEDs
  pinMode(3, OUTPUT); // FRONT LED

  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print(StrH);
  delay(2000);
  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("Let's begin");

  servo.attach(12);

  // заранее считаем окружность
  L = pi * D;

  // стартовое время для ISR
  lastPulseMs = millis();
}

void loop() {
  // --- безопасно читаем значения из ISR ---
  unsigned long localDeltaMs;
  unsigned long localRevCount;
  noInterrupts();
  localDeltaMs = deltaMs;
  localRevCount = revCount;
  interrupts();

  // пересчёт дистанции по числу оборотов
  distance_m = (float)localRevCount * L;
  distance_km = distance_m * 0.001f;

  // частота/скорость
  if (localDeltaMs > 0) {
    f = 1000.0f / (float)localDeltaMs; // Гц
  } else {
    f = 0.0f;
  }
  V = L * f;                   // м/с
  int V_kmh = (int)(V * 3.6f); // км/ч

  // датчик освещённости
  val = analogRead(RES);
  bool night = (val > 600); // чем больше val, тем темнее (если делитель так собран)

  // Вывод на LCD (минимум clear для снижения мерцания)
  lcd.clear();
  if (night) {
    lcd.setCursor(0, 0);
    lcd.print("  NIGHT MODE");
    lcd.setCursor(0, 1);
    lcd.print("Speed: ");
    lcd.print(V_kmh);
    lcd.print(" km/h");
  } else {
    lcd.setCursor(0, 0);
    lcd.print("Speed: ");
    lcd.print(V_kmh);
    lcd.print(" km/h");

    lcd.setCursor(0, 1);
    lcd.print("Distance:");
    lcd.print(distance_km, 3);
    lcd.print("km");
  }

  // LED логика (без ручного вызова ISR!)
  if (night) {
    digitalWrite(4, LOW);
    digitalWrite(3, LOW);
  } else {
    blinking();
  }

  // --- Servo ---
  // 0 км/ч → 180°, 45 км/ч → 0°
  int computed = 180 - (V_kmh * 180 / 45);
  if (computed < 0) computed = 0;
  if (computed > 180) computed = 180;
  angle = computed;
  servo.write(angle);

  delay(40);
}
