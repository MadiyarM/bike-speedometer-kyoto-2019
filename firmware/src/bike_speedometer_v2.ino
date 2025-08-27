#include <LiquidCrystal.h>
#include <Servo.h>

/* ---------- PINS ---------- */
const uint8_t PIN_INT   = 2;      // датчик/геркон (прерывание)
const uint8_t PIN_LED_F = 3;      // фронтальный LED
const uint8_t PIN_LED_B = 4;      // задние LEDs
const uint8_t PIN_SERVO = 12;     // серво
const uint8_t PIN_RES   = A0;     // фоторезистор (аналог)

/* ---------- LCD ---------- */
/* Вариант А: RW на пине 6 (как у вас сейчас)
   RS=5, RW=6, EN=7, D4=8, D5=9, D6=10, D7=11 */
LiquidCrystal lcd(5, 6, 7, 8, 9, 10, 11);

/* Если RW посажен на GND, замените строку выше на:
   LiquidCrystal lcd(5, 7, 8, 9, 10, 11);  // RS=5, EN=7, D4..D7=8..11
*/
const char StrH[] = "Hello!";

/* ---------- TIME / MOTION ---------- */
constexpr float PI_F = 3.14159f;
const float D = 0.67f;       // диаметр колеса (м)
float L = 0.0f;              // длина окружности (м)

// Данные из ISR — только целые и только volatile
volatile unsigned long lastPulseMs = 0;
volatile unsigned long deltaMs     = 0;
volatile unsigned long revCount    = 0;

/* Рабочие (не volatile) — считаем в loop() */
float f = 0.0f;              // Гц
float V = 0.0f;              // м/с
float distance_m = 0.0f;     // м
float distance_km = 0.0f;    // км

/* ---------- LED ---------- */
int val = 0;

/* ---------- SERVO ---------- */
Servo servo;
int angle = 0;

/* ---------- ISR: магнит обнаружен ---------- */
void magnet_detect() {
  unsigned long now = millis();

  // простой антидребезг ~10 мс
  if (now - lastPulseMs >= 10) {
    deltaMs = now - lastPulseMs;  // интервал между импульсами, мс
    lastPulseMs = now;
    revCount++;                   // считаем обороты; дистанцию посчитаем в loop()
  }
}

/* ---------- мигание светом ---------- */
void blinking() {
  digitalWrite(PIN_LED_B, HIGH);
  digitalWrite(PIN_LED_F, HIGH);
  delay(500);
  digitalWrite(PIN_LED_B, LOW);
  digitalWrite(PIN_LED_F, LOW);
  delay(100);
}

void setup() {
  Serial.begin(9600);

  pinMode(PIN_LED_B, OUTPUT);
  pinMode(PIN_LED_F, OUTPUT);
  pinMode(PIN_INT,   INPUT_PULLUP);

  lcd.begin(16, 2);

  // приветствие
  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print(StrH);
  delay(2000);
  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("Let's begin");
  delay(800);

  // подготовка измерений
  L = PI_F * D;
  lastPulseMs = millis();

  // прерывание по фронту геркона/датчика (активный LOW -> FALLING)
  attachInterrupt(digitalPinToInterrupt(PIN_INT), magnet_detect, FALLING);

  // серво
  servo.attach(PIN_SERVO);
}

void loop() {
  // ----- атомарно читаем значения из ISR -----
  unsigned long localDeltaMs;
  unsigned long localRevCount;
  noInterrupts();
  localDeltaMs = deltaMs;
  localRevCount = revCount;
  interrupts();

  // расчёт дистанции
  distance_m  = (float)localRevCount * L;
  distance_km = distance_m * 0.001f;

  // расчёт скорости
  if (localDeltaMs > 0) {
    f = 1000.0f / (float)localDeltaMs;  // Гц
  } else {
    f = 0.0f;
  }
  V = L * f;                             // м/с
  int V2 = (int)(V * 3.6f);              // км/ч

  // освещённость
  val = analogRead(PIN_RES);
  bool night = (val > 600);              // подстройте порог под вашу схему

  // ----- вывод на LCD (без лишних clear) -----
  lcd.clear();
  if (night) {
    lcd.setCursor(0, 0);
    lcd.print("  NIGHT MODE");
    lcd.setCursor(0, 1);
    lcd.print("Speed: ");
    lcd.print(V2);
    lcd.print(" km/h");
  } else {
    lcd.setCursor(0, 0);
    lcd.print("Speed: ");
    lcd.print(V2);
    lcd.print(" km/h");

    lcd.setCursor(0, 1);
    lcd.print("Distance:");
    lcd.print(distance_km, 3);  // 3 знака после запятой
    lcd.print("km");
  }

  // ----- свет -----
  if (night) {
    digitalWrite(PIN_LED_B, LOW);
    digitalWrite(PIN_LED_F, LOW);
  } else {
    blinking(); // как в исходной логике
  }

  // ----- серво-стрелка -----
  angle = 180 - (V2 * 180 / 45);
  if (angle < 0)   angle = 0;
  if (angle > 180) angle = 180;
  servo.write(angle);

  delay(40);
}
