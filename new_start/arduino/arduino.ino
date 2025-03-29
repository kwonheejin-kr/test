#include <Wire.h>

// Arduino 슬레이브 주소
#define SLAVE_ADDR 0x13

// 릴레이 핀 정의
#define FAN_PIN 2
#define LED_PIN 3
#define HUMIDIFIER_PIN 4

// 릴레이 제어 명령 정의
#define FAN_ON 'F'
#define FAN_OFF 'f'
#define LED_ON 'L'
#define LED_OFF 'l'
#define HUMIDIFIER_ON 'H'
#define HUMIDIFIER_OFF 'h'

// 릴레이 상태 변수
byte relayStatus = 0;

void setup() {
  // 시리얼 초기화
  Serial.begin(115200);
  Serial.println("Arduino R4 릴레이 제어 슬레이브 시작");
  
  // 릴레이 핀 초기화
  pinMode(FAN_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(HUMIDIFIER_PIN, OUTPUT);
  
  // 초기상태: 모든 릴레이 끄기
  digitalWrite(FAN_PIN, LOW);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(HUMIDIFIER_PIN, LOW);
  
  // I2C 초기화 (슬레이브 모드)
  Wire.begin(SLAVE_ADDR);
  
  // 이벤트 핸들러 등록
  Wire.onReceive(receiveEvent); // 마스터로부터 데이터 수신 시 호출
  Wire.onRequest(requestEvent); // 마스터가 데이터 요청 시 호출
  
  // I2C 슬레이브 주소 출력
  Serial.print("I2C 슬레이브 주소: 0x");
  Serial.println(SLAVE_ADDR, HEX);
  Serial.println("I2C 슬레이브 초기화 완료");
}

void loop() {
  // 릴레이 상태 출력 (5초마다)
  static unsigned long lastStatusPrint = 0;
  if (millis() - lastStatusPrint > 5000) {
    printStatus();
    lastStatusPrint = millis();
  }
  
  delay(100);
}

// I2C 데이터 수신 이벤트 핸들러
void receiveEvent(int byteCount) {
  Serial.print("데이터 수신 이벤트 발생! 수신 바이트 수: ");
  Serial.println(byteCount);
  
  while (Wire.available()) {
    byte command = Wire.read();  // 모든 명령은 바이트로 받음
    Serial.print("수신 명령: ");
    
    // ASCII 문자와 숫자값 모두 출력
    if (command >= 32 && command <= 126) {  // 출력 가능한 ASCII 문자라면
      Serial.write(command);  // 문자로 출력
    }
    
    Serial.print(" (값: ");
    Serial.print(command);
    Serial.println(")");
    
    // 숫자 값 처리 (1, 0)
    if (command == 1) {
      digitalWrite(LED_PIN, HIGH);
      Serial.println("LED 켜기 (숫자 1 명령)");
      // 릴레이 상태 변경
      relayStatus = relayStatus | 0b00000010;  // LED 비트 설정
    } 
    else if (command == 0) {
      digitalWrite(LED_PIN, LOW);
      Serial.println("LED 끄기 (숫자 0 명령)");
      // 상태 변수 업데이트
      relayStatus = relayStatus & ~0b00000010;  // LED 비트 클리어
    }
    // 문자 명령 처리 ('F', 'f', 'L', 'l', ...)
    else if (command == 'F' || command == 'f' || 
             command == 'L' || command == 'l' || 
             command == 'H' || command == 'h') {
      processCommand((char)command);  // 문자로 변환하여 처리
    }
    else {
      Serial.print("알 수 없는 명령 값: ");
      Serial.println(command);
    }
  }
}

// I2C 데이터 요청 이벤트 핸들러
void requestEvent() {
  Wire.write(relayStatus);
}

// 명령 처리 함수
void processCommand(char command) {
  Serial.print("수신 명령: ");
  Serial.println(command);
  
  switch (command) {
    case FAN_ON:
      setFan(true);
      break;
    case FAN_OFF:
      setFan(false);
      break;
    case LED_ON:
      setLed(true);
      break;
    case LED_OFF:
      setLed(false);
      break;
    case HUMIDIFIER_ON:
      setHumidifier(true);
      break;
    case HUMIDIFIER_OFF:
      setHumidifier(false);
      break;
    default:
      Serial.println("잘못된 명령");
      break;
  }
}

// 팬 제어 함수
void setFan(bool state) {
  digitalWrite(FAN_PIN, state ? HIGH : LOW);
  if (state) {
    relayStatus |= 0x01;  // 비트 0: 팬 제어
  } else {
    relayStatus &= ~0x01; // 비트 0: 팬 제어
  }
  Serial.print("팬 ");
  Serial.println(state ? "켜짐" : "꺼짐");
}

// LED 제어 함수
void setLed(bool state) {
  digitalWrite(LED_PIN, state ? HIGH : LOW);
  if (state) {
    relayStatus |= 0x02;  // 비트 1: LED 제어
  } else {
    relayStatus &= ~0x02; // 비트 1: LED 제어
  }
  Serial.print("LED ");
  Serial.println(state ? "켜짐" : "꺼짐");
}

// 가습기 제어 함수
void setHumidifier(bool state) {
  digitalWrite(HUMIDIFIER_PIN, state ? HIGH : LOW);
  if (state) {
    relayStatus |= 0x04;  // 비트 2: 가습기 제어
  } else {
    relayStatus &= ~0x04; // 비트 2: 가습기 제어
  }
  Serial.print("가습기 ");
  Serial.println(state ? "켜짐" : "꺼짐");
}

// 릴레이 상태 출력 함수
void printStatus() {
  Serial.println("현재 릴레이 상태:");
  Serial.print("팬: ");
  Serial.println((relayStatus & 0x01) ? "켜짐" : "꺼짐");
  Serial.print("LED: ");
  Serial.println((relayStatus & 0x02) ? "켜짐" : "꺼짐");
  Serial.print("가습기: ");
  Serial.println((relayStatus & 0x04) ? "켜짐" : "꺼짐");
  Serial.print("상태 비트: ");
  Serial.println(relayStatus, BIN);
}
