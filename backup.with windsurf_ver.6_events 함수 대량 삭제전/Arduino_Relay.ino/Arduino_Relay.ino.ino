// Arduino_Relay.ino
// 펌웨어 버전: 1.0.0 (2025-03-26)
// ESP32 SmartFarm 프로젝트의 릴레이 제어 모듈

#include <Wire.h>

#define SLAVE_ADDRESS 0x08
#define FAN_PIN 2     // D2: 팬 릴레이
#define LED_PIN 3     // D3: LED 릴레이
#define HUM_PIN 4     // D4: 가습기 릴레이

// 릴레이 상태 변수
byte relay_status = 0;

void setup() {
  // I2C 슬레이브 초기화
  Wire.begin(SLAVE_ADDRESS);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
  
  // 릴레이 핀 설정
  pinMode(FAN_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(HUM_PIN, OUTPUT);
  
  // 초기 상태: 모든 릴레이 OFF
  digitalWrite(FAN_PIN, LOW);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(HUM_PIN, LOW);
  
  Serial.begin(9600);  // 디버깅용
  Serial.println("Arduino Relay Control 초기화 완료");
}

void loop() {
  // 상태 업데이트
  relay_status = 0;
  relay_status |= (digitalRead(LED_PIN) << 4);  // 상위 4비트: LED
  relay_status |= (digitalRead(FAN_PIN) << 2);  // 중간 2비트: 팬
  relay_status |= digitalRead(HUM_PIN);         // 하위 2비트: 가습기
  
  delay(100);  // 약간의 지연
}

// I2C로 명령 수신
void receiveEvent(int bytes) {
  if (Wire.available()) {
    char command = Wire.read();
    
    switch(command) {
      case 'F':  // Fan ON
        digitalWrite(FAN_PIN, HIGH);
        Serial.println("Fan ON");
        break;
      case 'f':  // Fan OFF
        digitalWrite(FAN_PIN, LOW);
        Serial.println("Fan OFF");
        break;
      case 'L':  // LED ON
        digitalWrite(LED_PIN, HIGH);
        Serial.println("LED ON");
        break;
      case 'l':  // LED OFF
        digitalWrite(LED_PIN, LOW);
        Serial.println("LED OFF");
        break;
      case 'H':  // Humidifier ON
        digitalWrite(HUM_PIN, HIGH);
        Serial.println("Humidifier ON");
        break;
      case 'h':  // Humidifier OFF
        digitalWrite(HUM_PIN, LOW);
        Serial.println("Humidifier OFF");
        break;
    }
  }
}

// 상태 요청에 응답
void requestEvent() {
  Wire.write(relay_status);
}