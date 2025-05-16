#include <Arduino.h>
#include <PinChangeInterrupt.h>

const int CH7_PIN = A0;
const int CH3_PIN = A1;
const int CH2_PIN = A2;

const int LED_ONOFF_PIN  = 2;
const int LED_BRIGHT_PIN = 3;
const int LED_R_PIN      = 4;
const int LED_G_PIN      = 5;
const int LED_B_PIN      = 6;

volatile unsigned long rise7 = 0, rise3 = 0, rise2 = 0;
volatile unsigned int  w7 = 1500, w3 = 1500, w2 = 1500;
volatile bool          ch7_upd = false, ch3_upd = false, ch2_upd = false;

void ISR_ch7() {
  if (digitalRead(CH7_PIN)) {
    rise7 = micros();
  } else {
    unsigned long dt = micros() - rise7;
    w7 = (dt >= 1000 && dt <= 2000) ? dt : 1500;
    ch7_upd = true;
  }
}

void ISR_ch3() {
  if (digitalRead(CH3_PIN)) {
    rise3 = micros();
  } else {
    unsigned long dt = micros() - rise3;
    w3 = (dt >= 1000 && dt <= 2000) ? dt : 1500;
    ch3_upd = true;
  }
}

void ISR_ch2() {
  if (digitalRead(CH2_PIN)) {
    rise2 = micros();
  } else {
    unsigned long dt = micros() - rise2;
    w2 = (dt >= 1000 && dt <= 2000) ? dt : 1500;
    ch2_upd = true;
  }
}

void hsvToRgb(float h, float s, float v, int &r, int &g, int &b) {
  float c = v * s;
  float x = c * (1 - fabs(fmod(h / 60.0, 2) - 1));
  float m = v - c;
  float r1, g1, b1;
  if      (h <  60) { r1 = c;  g1 = x;  b1 = 0; }
  else if (h < 120) { r1 = x;  g1 = c;  b1 = 0; }
  else if (h < 180) { r1 = 0;  g1 = c;  b1 = x; }
  else if (h < 240) { r1 = 0;  g1 = x;  b1 = c; }
  else if (h < 300) { r1 = x;  g1 = 0;  b1 = c; }
  else              { r1 = c;  g1 = 0;  b1 = x; }
  r = (r1 + m) * 255;
  g = (g1 + m) * 255;
  b = (b1 + m) * 255;
}

void setup() {
  pinMode(CH7_PIN, INPUT_PULLUP);
  pinMode(CH3_PIN, INPUT_PULLUP);
  pinMode(CH2_PIN, INPUT_PULLUP);
  pinMode(LED_ONOFF_PIN,  OUTPUT);
  pinMode(LED_BRIGHT_PIN, OUTPUT);
  pinMode(LED_R_PIN,      OUTPUT);
  pinMode(LED_G_PIN,      OUTPUT);
  pinMode(LED_B_PIN,      OUTPUT);
  attachPCINT(digitalPinToPCINT(CH7_PIN), ISR_ch7, CHANGE);
  attachPCINT(digitalPinToPCINT(CH3_PIN), ISR_ch3, CHANGE);
  attachPCINT(digitalPinToPCINT(CH2_PIN), ISR_ch2, CHANGE);
  Serial.begin(115200);
}

void loop() {
  noInterrupts();
  unsigned int pw7 = w7;
  unsigned int pw3 = w3;
  unsigned int pw2 = w2;
  bool u7 = ch7_upd, u3 = ch3_upd, u2 = ch2_upd;
  ch7_upd = ch3_upd = ch2_upd = false;
  interrupts();

  digitalWrite(LED_ONOFF_PIN, pw7 > 1500 ? HIGH : LOW);

  int bright = constrain(map(pw3, 1000, 2000, 0, 255), 0, 255);
  if (bright <= 20) digitalWrite(LED_BRIGHT_PIN, LOW);
  else              analogWrite(LED_BRIGHT_PIN, bright);

  float hue = map(pw2, 1000, 2000, 0, 360);
  int r, g, b;
  hsvToRgb(hue, 1.0, 1.0, r, g, b);
  analogWrite(LED_R_PIN, 255 - r);
  analogWrite(LED_G_PIN, 255 - g);
  analogWrite(LED_B_PIN, 255 - b);

  if (u7 || u3 || u2) {
    Serial.print("CH7="); Serial.print(pw7);
    Serial.print(" CH3="); Serial.print(pw3);
    Serial.print(" CH2="); Serial.print(pw2);
    Serial.print(" Hue="); Serial.print(hue);
    Serial.print(" BrightLED="); Serial.println(bright);
  }

  delay(20);
}
