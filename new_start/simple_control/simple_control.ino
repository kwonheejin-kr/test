#include <Wire.h>

// I2C 핀 정의
#define SDA_PIN 21
#define SCL_PIN 22

// 아두이노 슬레이브 주소
#define ARDUINO_ADDR 0x08

// 릴레이 제어 명령 정의
#define FAN_ON 'F'
#define FAN_OFF 'f'
#define LED_ON 'L'
#define LED_OFF 'l'
#define HUMIDIFIER_ON 'H'
#define HUMIDIFIER_OFF 'h'

// 릴레이 상태 변수
bool fanStatus = false;
bool ledStatus = false;
bool humidifierStatus = false;

void setup() {
  // 시리얼 초기화
  Serial.begin(115200);
  while (!Serial) delay(10);  // 시리얼 연결 대기
  
  Serial.println("\nESP32 릴레이 간편 제어 프로그램 v1.0.0");
  Serial.println("-------------------------------");
  Serial.println("제어 방법: 숫자 키를 입력하세요");
  Serial.println("1: 팬 켜기    2: 팬 끄기");
  Serial.println("3: LED 켜기   4: LED 끄기");
  Serial.println("5: 가습기 켜기 6: 가습기 끄기");
  Serial.println("s: 상태 보기  h: 도움말");
  Serial.println("-------------------------------");
  
  // I2C 초기화
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(100000);  // I2C 속도를 100kHz로 제한 (아두이노 호환성)
  
  // 아두이노 연결 확인
  if (checkArduinoConnection()) {
    Serial.println("아두이노 연결됨: 준비 완료!");
    // 아두이노 상태 확인
    checkRelayStatus();
  } else {
    Serial.println("아두이노 연결 안됨: 연결을 확인하세요!");
  }
}

void loop() {
  // 시리얼 명령 처리
  if (Serial.available() > 0) {
    char key = Serial.read();
    processKey(key);
  }
  
  delay(50);  // 약간의 딜레이
}

// 키 처리 함수
void processKey(char key) {
  switch (key) {
    case '1':  // 팬 켜기
      sendCommand(FAN_ON);
      fanStatus = true;
      Serial.println("명령: 팬 켜기");
      break;
      
    case '2':  // 팬 끄기
      sendCommand(FAN_OFF);
      fanStatus = false;
      Serial.println("명령: 팬 끄기");
      break;
      
    case '3':  // LED 켜기
      sendCommand(LED_ON);
      ledStatus = true;
      Serial.println("명령: LED 켜기");
      break;
      
    case '4':  // LED 끄기
      sendCommand(LED_OFF);
      ledStatus = false;
      Serial.println("명령: LED 끄기");
      break;
      
    case '5':  // 가습기 켜기
      sendCommand(HUMIDIFIER_ON);
      humidifierStatus = true;
      Serial.println("명령: 가습기 켜기");
      break;
      
    case '6':  // 가습기 끄기
      sendCommand(HUMIDIFIER_OFF);
      humidifierStatus = false;
      Serial.println("명령: 가습기 끄기");
      break;
      
    case 's':  // 상태 확인
    case 'S':
      checkRelayStatus();
      break;
      
    case 'h':  // 도움말
    case 'H':
    case '?':
      printHelp();
      break;
      
    case '\n':  // 줄바꿈 무시
    case '\r':
      break;
      
    default:
      Serial.println("잘못된 키 입력: '" + String(key) + "'");
      Serial.println("도움말은 'h'를 입력하세요.");
      break;
  }
}

// 아두이노로 명령 전송 함수
void sendCommand(char command) {
  // 명령 전송 시도 3회
  int attempts = 0;
  byte error = 4;  // 초기값은 오류
  
  while (attempts < 3 && error != 0) {
    Wire.beginTransmission(ARDUINO_ADDR);
    Wire.write(command);
    error = Wire.endTransmission();
    
    if (error == 0) {
      Serial.println("명령 전송 성공!");
      break;
    } else {
      attempts++;
      Serial.print("전송 실패 (" + String(attempts) + "/3), 오류 코드: ");
      Serial.println(error);
      delay(50);  // 재시도 전 약간의 딜레이
    }
  }
  
  if (error != 0) {
    Serial.println("전송 실패: 아두이노 연결을 확인하세요.");
  }
  
  delay(100);  // 명령 처리 시간 확보
}

// 아두이노 연결 확인 함수
bool checkArduinoConnection() {
  Wire.beginTransmission(ARDUINO_ADDR);
  byte error = Wire.endTransmission();
  
  if (error == 0) {
    return true;
  } else {
    Serial.print("아두이노 연결 오류 코드: ");
    Serial.println(error);
    return false;
  }
}

// 릴레이 상태 확인 함수
void checkRelayStatus() {
  Serial.println("-------------------------------");
  Serial.println("현재 릴레이 상태 확인 중...");
  
  Wire.requestFrom(ARDUINO_ADDR, 1);  // 1바이트 요청
  
  if (Wire.available()) {
    byte status = Wire.read();
    
    // 상태 출력
    Serial.print("상태 바이트: ");
    Serial.println(status, BIN);
    
    // 각 릴레이 상태 파악
    fanStatus = (status & 0x01);
    ledStatus = (status & 0x02);
    humidifierStatus = (status & 0x04);
    
    // 표시
    Serial.print("팬: ");
    Serial.println(fanStatus ? "켜짐" : "꺼짐");
    Serial.print("LED: ");
    Serial.println(ledStatus ? "켜짐" : "꺼짐");
    Serial.print("가습기: ");
    Serial.println(humidifierStatus ? "켜짐" : "꺼짐");
  } else {
    Serial.println("아두이노에서 응답이 없습니다.");
    Serial.println("연결을 확인하세요.");
  }
  
  Serial.println("-------------------------------");
}

// 도움말 표시 함수
void printHelp() {
  Serial.println("-------------------------------");
  Serial.println("중요: 숫자나 문자만 입력하세요");
  Serial.println("1: 팬 켜기    2: 팬 끄기");
  Serial.println("3: LED 켜기   4: LED 끄기");
  Serial.println("5: 가습기 켜기 6: 가습기 끄기");
  Serial.println("s: 상태 보기  h: 도움말");
  Serial.println("-------------------------------");
}