## 🎥 이번주 유튭

🙉수아짱🙉: https://youtube.com/shorts/04p9sRpWkyo?si=PFVF2Mbwo5zwFvU6

🐼채은짱🐼: https://youtube.com/shorts/xD_c_B3hmJ0?si=-x8yrGIUkRA-vTfy

---

## 📌 개요

본 프로젝트는 RC 수신기(RD9)에서 나오는 PWM 신호 3채널을 읽어  
1. LED On/Off  
2. LED 밝기 제어  
3. 3색(RGB) LED 색 변환  

PWM 디코딩에는 PinChangeInterrupt 라이브러리를 사용했고,  
HSV→RGB 변환 알고리즘을 통해 컬러 LED를 제어

---

## 🎯 기능

1. **LED On/Off**  
   - CH7(PW7) 신호가 1500µs 초과 시 LED 켜기, 이하 시 LED 끄기  
2. **LED 밝기 제어**  
   - CH3(PW3) 신호(1000–2000µs) 범위를 0–255 PWM으로 매핑하여 밝기 조절  
3. **3색 LED 색 변환**  
   - CH2(PW2) 신호를 0–360도 Hue로 매핑 후 HSV→RGB 변환하여 색상 변경  

---

## 🛠️ 하드웨어 구성

- **RC 수신기**: RD9  
- **마이크로컨트롤러**: Arduino Uno (5V)  
- **LED**:  
  - 단일색 LED ×2 (On/Off 표시, 밝기 제어)  
  - 3색(Common Anode) RGB LED ×1  

---

## 🔌 회로 연결

| 신호          | RC 채널  | Arduino 핀      | 용도                         |
|:-------------:|:--------:|:---------------:|:----------------------------:|
| CH7 (On/Off)  | 채널 7   | A0 (PCINT8)     | LED On/Off 제어              |
| CH3 (Brightness)| 채널 3 | A1 (PCINT9)     | 단일 LED 밝기 제어           |
| CH2 (Hue)     | 채널 2   | A2 (PCINT10)    | RGB LED 색상(Hue) 제어       |
| On/Off LED    | —        | D2              | 디지털 출력 (LOW: 꺼짐, HIGH: 켜짐) |
| Brightness LED| —        | D3              | PWM 출력 (0–255 밝기)        |
| RGB LED (R)   | —        | D4              | 빨간색 채널 (공통 애노드)    |
| RGB LED (G)   | —        | D5              | 초록색 채널 (공통 애노드)    |
| RGB LED (B)   | —        | D6              | 파란색 채널 (공통 애노드)    |
| RGB LED 애노드| —        | +5V (저항 필요)| 공통 애노드                   |

|브래드보드|아두이노|RC 수신기|
|----|-----|-----|
|![image](https://github.com/user-attachments/assets/94c9c796-a4e7-4069-9125-3d08ad278159)|![image](https://github.com/user-attachments/assets/3279b4a1-366e-4aa2-8a5c-f13332e573f5)|![image](https://github.com/user-attachments/assets/e6908f15-a8ca-4ddd-bd72-b2f27ca07f93)|

---

## 👑 역할 분담

capturePluse, hsvToRgb 함수 => 🐼채은짱🐼

나머지 코드 => 🙉수아짱🙉

