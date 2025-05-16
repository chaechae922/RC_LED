#include <Arduino.h>
#include <PinChangeInterrupt.h>

// 채널, 핀 정의 
const int CH7_PIN = A0;
const int CH3_PIN = A1;  
const int CH2_PIN = A2; 
const int LED_ONOFF_PIN  = 2;
const int LED_BRIGHT_PIN = 3;
const int LED_R_PIN      = 4;
const int LED_G_PIN      = 5;
const int LED_B_PIN      = 6;

// 전역(인터럽트 공유) 변수
volatile unsigned long rise7 = 0, rise3 = 0, rise2 = 0;
volatile unsigned int  pw7   = 1500, pw3   = 1500, pw2   = 1500;
volatile bool          up7   = false, up3   = false, up2   = false;

// 인터럽트(ISR)
inline void capturePulse(volatile unsigned long &rise,
                         volatile unsigned int &pw,
                         volatile bool &flag,
                         int pin)
{
  if (digitalRead(pin)) {
    rise = micros();                 // 상승
  } else {
    unsigned long dt = micros() - rise;
    pw   = (dt >= 1000 && dt <= 2000) ? dt : 1500;
    flag = true;                     // 새 값 표시
  }
}

void ISR_ch7() { capturePulse(rise7, pw7, up7, CH7_PIN); }
void ISR_ch3() { capturePulse(rise3, pw3, up3, CH3_PIN); }
void ISR_ch2() { capturePulse(rise2, pw2, up2, CH2_PIN); }

// HSV -> RGB 변환
void hsvToRgb(float h, float s, float v, int &r, int &g, int &b)
{
  float c = v * s;
  float x = c * (1 - fabs(fmod(h / 60.0f, 2) - 1));
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


// 초기 설정
void setup()
{
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

// 메인 루프
void loop()
{
  // 1) 인터럽트 값 스냅샷
  noInterrupts();
    unsigned int p7 = pw7;
    unsigned int p3 = pw3;
    unsigned int p2 = pw2;
    up7 = up3 = up2 = false;
  interrupts();

  // 2) ON/OFF LED (D2)
  digitalWrite(LED_ONOFF_PIN, p7 > 1500 ? HIGH : LOW);

  // 3) 밝기 LED (D3)
  int brightness = constrain(map(p3, 1000, 2000, 0, 255), 0, 255);
  if (brightness <= 20) digitalWrite(LED_BRIGHT_PIN, LOW);
  else                  analogWrite(LED_BRIGHT_PIN, brightness);

  // 4) RGB LED (D4,5,6) 
  float hue = map(p2, 1000, 2000, 0, 360);
  int r, g, b;
  hsvToRgb(hue, 1.0f, 1.0f, r, g, b);
  analogWrite(LED_R_PIN, 255 - r);
  analogWrite(LED_G_PIN, 255 - g);
  analogWrite(LED_B_PIN, 255 - b);

  delay(20);  
}
