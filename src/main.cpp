#include <Arduino.h>
#include <PinChangeInterrupt.h>

// ───── 채널·핀 정의 ───────────────────────────────────
const int CH7_PIN = A0;   // PWM 채널 7 입력 (On/Off)
const int CH3_PIN = A1;   // PWM 채널 3 입력 (밝기)
const int CH2_PIN = A2;   // PWM 채널 2 입력 (색상 Hue)
const int LED_ONOFF_PIN  = 2;  // 단일색 LED On/Off 제어 핀
const int LED_BRIGHT_PIN = 3;  // 단일색 LED 밝기 제어(PWM) 핀
const int LED_R_PIN      = 4;  // RGB LED 빨강 채널 (공통 애노드)
const int LED_G_PIN      = 5;  // RGB LED 초록 채널 (공통 애노드)
const int LED_B_PIN      = 6;  // RGB LED 파랑 채널 (공통 애노드)

// ───── 전역(인터럽트 공유) 변수 ───────────────────────
// rise: 상승 에지 시간 기록, pw: 펄스폭, flag: 신규 값 표시 플래그
volatile unsigned long rise7 = 0, rise3 = 0, rise2 = 0;
volatile unsigned int  pw7   = 1500, pw3   = 1500, pw2   = 1500;
volatile bool          up7   = false, up3   = false, up2   = false;

// ───── 인터럽트용 공통 캡처 함수 ───────────────────────
// PWM 입력 핀에서 CHANGE 인터럽트 발생 시 호출.
// HIGH->LOW 전환 시 펄스폭 계산하고, 1000–2000µs 범위면 pw에 저장.
inline void capturePulse(volatile unsigned long &rise,
                         volatile unsigned int &pw,
                         volatile bool &flag,
                         int pin)
{
  if (digitalRead(pin)) {
    // 상승 에지: 시간 측정 시작
    rise = micros();
  } else {
    // 하강 에지: 펄스폭 계산
    unsigned long dt = micros() - rise;
    // 유효 범위(1ms–2ms) 검사, 아니면 기본값 1500µs
    pw   = (dt >= 1000 && dt <= 2000) ? dt : 1500;
    flag = true;  // 새로운 값 도착 표시
  }
}
void ISR_ch7() { capturePulse(rise7, pw7, up7, CH7_PIN); }  // 채널7 ISR
void ISR_ch3() { capturePulse(rise3, pw3, up3, CH3_PIN); }  // 채널3 ISR
void ISR_ch2() { capturePulse(rise2, pw2, up2, CH2_PIN); }  // 채널2 ISR

// ───── HSV → RGB 변환 함수 ─────────────────────────────
// h: 0–360°, s,v: 0.0–1.0 → r,g,b: 0–255
void hsvToRgb(float h, float s, float v, int &r, int &g, int &b)
{
  float c = v * s;
  float x = c * (1 - fabs(fmod(h / 60.0f, 2) - 1));
  float m = v - c;
  float r1, g1, b1;

  // Hue 구간별 1차 RGB 계산
  if      (h <  60) { r1 = c;  g1 = x;  b1 = 0; }
  else if (h < 120) { r1 = x;  g1 = c;  b1 = 0; }
  else if (h < 180) { r1 = 0;  g1 = c;  b1 = x; }
  else if (h < 240) { r1 = 0;  g1 = x;  b1 = c; }
  else if (h < 300) { r1 = x;  g1 = 0;  b1 = c; }
  else              { r1 = c;  g1 = 0;  b1 = x; }

  // m 더해 최종 RGB 계산
  r = (r1 + m) * 255;
  g = (g1 + m) * 255;
  b = (b1 + m) * 255;
}

// ───── 초기 설정 ─────────────────────────────────────
void setup()
{
  // PWM 입력 채널 핀 풀업 모드로 설정
  pinMode(CH7_PIN, INPUT_PULLUP);
  pinMode(CH3_PIN, INPUT_PULLUP);
  pinMode(CH2_PIN, INPUT_PULLUP);

  // LED 출력 핀 설정
  pinMode(LED_ONOFF_PIN,  OUTPUT);
  pinMode(LED_BRIGHT_PIN, OUTPUT);
  pinMode(LED_R_PIN,      OUTPUT);
  pinMode(LED_G_PIN,      OUTPUT);
  pinMode(LED_B_PIN,      OUTPUT);

  // PinChangeInterrupt 라이브러리로 ISR 할당
  attachPCINT(digitalPinToPCINT(CH7_PIN), ISR_ch7, CHANGE);
  attachPCINT(digitalPinToPCINT(CH3_PIN), ISR_ch3, CHANGE);
  attachPCINT(digitalPinToPCINT(CH2_PIN), ISR_ch2, CHANGE);

  Serial.begin(115200);  // 상태 모니터링용 시리얼 시작
}

// ───── 메인 루프 ─────────────────────────────────────
void loop()
{
  // --- 1) 인터럽트 공유 변수 스냅샷 가져오기 ---
  noInterrupts();
    unsigned int p7 = pw7;   // On/Off 채널
    unsigned int p3 = pw3;   // 밝기 채널
    unsigned int p2 = pw2;   // 색상 채널
    // up7, up3, up2 플래그 초기화 (사용 안 함 시 무시)
    up7 = up3 = up2 = false;
  interrupts();

  // --- 2) LED On/Off 제어 (채널7) ---
  // 펄스폭 > 1500µs 이면 HIGH, 아니면 LOW
  digitalWrite(LED_ONOFF_PIN, p7 > 1500 ? HIGH : LOW);

  // --- 3) LED 밝기 제어 (채널3 → PWM) ---
  // 1000–2000µs 범위를 0–255 밝기로 매핑
  int brightness = constrain(map(p3, 1000, 2000, 0, 255), 0, 255);
  if (brightness <= 20) {
    // 너무 어두우면 완전 끔
    digitalWrite(LED_BRIGHT_PIN, LOW);
  } else {
    // PWM 출력으로 밝기 조절
    analogWrite(LED_BRIGHT_PIN, brightness);
  }

  // --- 4) RGB LED 색상 제어 (채널2 → Hue) ---
  // 1000–2000µs 범위를 0–360° Hue 값으로 변환
  float hue = map(p2, 1000, 2000, 0, 360);
  int r, g, b;
  hsvToRgb(hue, 1.0f, 1.0f, r, g, b);  
  // Common Anode 방식: 255–계산값 으로 반전 출력
  analogWrite(LED_R_PIN, 255 - r);
  analogWrite(LED_G_PIN, 255 - g);
  analogWrite(LED_B_PIN, 255 - b);

  // --- 5) 반복 주기 조절 (50Hz) ---
  delay(20);
}
