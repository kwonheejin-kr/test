#include <Wire.h>
#include "Adafruit_SHT31.h"
#include "MAX30105.h"
#include "heartRate.h"
#include "spo2_algorithm.h"


// I2C 핀 정의
#define I2C_SDA 21
#define I2C_SCL 22

// Arduino 슬레이브 주소
#define SLAVE_ADDR 0x13

// 릴레이 제어 명령 정의
#define FAN_ON 'F'
#define FAN_OFF 'f'
#define LED_ON 'L'
#define LED_OFF 'l'
#define HUMIDIFIER_ON 'H'
#define HUMIDIFIER_OFF 'h'

// 센서 객체 생성
Adafruit_SHT31 sht31 = Adafruit_SHT31();
MAX30105 particleSensor;

// 센서 데이터 변수 - 이름 변경하여 충돌 방지
float temperature = 0.0;
float humidity = 0.0;
int32_t heartRateValue = 0;  // heartRate에서 이름 변경
int32_t spo2Value = 0;       // spo2에서 이름 변경
bool fanStatus = false;
bool ledStatus = false;
bool humidifierStatus = false;

// I2C 스캐너 모드 활성화 여부
bool scanI2C = false;

// I2C 버스가 사용 중인지 확인하는 변수
bool i2cBusy = false;

void setup() {
  // 시리얼 초기화
  Serial.begin(115200);
  Serial.println("ESP32 SmartFarm 프로젝트 시작");
  
  // I2C 초기화 (마스터 모드)
  Wire.begin(I2C_SDA, I2C_SCL, true);  // 내부 풀업 저항 사용
  Wire.setClock(100000);  // I2C 속도를 100kHz로 통일 (MAX30102 권장 속도)
  Serial.println("I2C 초기화 완료 (100kHz)");
  
  // I2C 초기화 후 지연시간 추가
  delay(100);

  if (scanI2C) {
    // I2C 스캐너 모드
    Serial.println("I2C 스캐너 모드 활성화");
    Serial.println("연결된 I2C 장치 스캔 중...");
    scanI2CDevices();
  } else {
    // 일반 모드 - 센서 초기화
    initializeSensors();  // 센서 초기화 함수로 분리
  }
}

// 센서 초기화 함수 - 코드 모듈화
void initializeSensors() {
  bool sht31Initialized = false;
  bool max30102Initialized = false;
  
  // I2C 장치 확인
  Serial.println("I2C 장치 확인 중...");
  byte error = 0;
  Wire.beginTransmission(SLAVE_ADDR);  // 아두이노 주소
  error = Wire.endTransmission();
  
  if (error == 0) {
    Serial.println("아두이노(0x13) 확인: 성공");
  } else {
    Serial.println("아두이노(0x13) 확인 실패! 오류 코드: " + String(error));
  }
  
  delay(50); // 요청 사이 안정화 시간
  
  // SHT31 온습도 센서 초기화
  for (int retry = 0; retry < 3; retry++) {
    if (sht31.begin(0x44)) {
      Serial.println("SHT31 센서 초기화 성공");
      sht31Initialized = true;
      break;
    } else {
      Serial.print("SHT31 센서 초기화 실패 " + String(retry+1) + "/3... ");
      if (retry < 2) {
        Serial.println("재시도 중");
        delay(100);
      } else {
        Serial.println("SHT31 센서 초기화 실패!");
      }
    }
  }
  
  delay(50); // 요청 사이 안정화 시간
  
  // MAX30102 심박/SpO2 센서 초기화
  for (int retry = 0; retry < 3; retry++) {
    if (particleSensor.begin(Wire, I2C_SPEED_STANDARD)) {
      Serial.println("MAX30102 센서 초기화 성공");
      // 심박수 측정 설정
      particleSensor.setup();
      particleSensor.setPulseAmplitudeRed(0x0A);
      particleSensor.setPulseAmplitudeGreen(0);
      max30102Initialized = true;
      break;
    } else {
      Serial.print("MAX30102 센서 초기화 실패 " + String(retry+1) + "/3... ");
      if (retry < 2) {
        Serial.println("재시도 중");
        delay(100);
      } else {
        Serial.println("MAX30102 센서 초기화 실패!");
      }
    }
  }
  
  // 초기화 상태 요약
  Serial.println("\n센서 초기화 상태:");
  Serial.println("- SHT31 온습도: " + String(sht31Initialized ? "성공" : "실패"));
  Serial.println("- MAX30102 심박/SpO2: " + String(max30102Initialized ? "성공" : "실패"));
  
  // 초기 상태 확인
  delay(50);
  checkRelayStatus();
}

void loop() {
  // I2C 스캐너 모드일 경우 루프 내에서는 아무 작업도 하지 않음
  if (scanI2C) {
    delay(1000);
    return;
  }
  
  // 일반 모드 - 센서 데이터는 명령어로만 읽기
  static unsigned long lastCommandCheck = 0;
  
  // 현재 시간 가져오기
  unsigned long currentMillis = millis();
  
  // 온습도 센서 자동 읽기 비활성화 (명령어로만 읽기 위함)
  
  // 시리얼 명령 처리 (200ms마다 확인)
  if (currentMillis - lastCommandCheck >= 200) {
    handleSerialCommands();
    lastCommandCheck = currentMillis;
  }
  
  // 루프 내 지연시간 추가
  delay(50);
}

// I2C 스캐너 함수
void scanI2CDevices() {
  byte error, address;
  int nDevices = 0;
  
  Serial.println("I2C 스캔 시작...");
  
  for(address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    
    if (error == 0) {
      Serial.print("I2C 장치 발견: 0x");
      if (address < 16) {
        Serial.print("0");
      }
      Serial.println(address, HEX);
      nDevices++;
    }
  }
  
  if (nDevices == 0) {
    Serial.println("I2C 장치를 찾을 수 없습니다");
  } else {
    Serial.println("스캔 완료: " + String(nDevices) + "개 장치 발견");
  }
  Serial.println();
}

// 온습도 센서 데이터 읽기 함수
void readTemperatureHumidity() {
  if (i2cBusy) {
    Serial.println("I2C 버스 사용 중. 잠시 후 다시 시도하세요.");
    return;
  }
  
  i2cBusy = true;
  
  float t = sht31.readTemperature();
  float h = sht31.readHumidity();
  
  if (!isnan(t)) {
    temperature = t;
  } else {
    Serial.println("온도 읽기 실패!");
  }
  
  if (!isnan(h)) {
    humidity = h;
  } else {
    Serial.println("습도 읽기 실패!");
  }
  
  Serial.print("온도: ");
  Serial.print(temperature);
  Serial.print("°C, 습도: ");
  Serial.print(humidity);
  Serial.println("%");
  
  i2cBusy = false;
}

// 심박수 측정 함수 - 라이브러리 함수 충돌 해결
void measureHeartRate() {
  if (i2cBusy) {
    Serial.println("I2C 버스 사용 중. 잠시 후 다시 시도하세요.");
    return;
  }
  
  i2cBusy = true;
  
  // 5초 동안 데이터 수집
  const byte RATE_SIZE = 4; // 평균 계산에 사용할 샘플 수
  byte rates[RATE_SIZE]; // 심박수 배열
  byte rateSpot = 0;
  long lastBeat = 0; // 마지막 심박 시간
  int beatAvg = 0; // 평균 심박수
  int spo2Measured = 0; // 산소포화도 값
  
  Serial.println("심박수 측정 시작...");
  
  long startTime = millis();
  while (millis() - startTime < 5000) {
    long irValue = particleSensor.getIR();
    
    if (irValue > 50000) {
      // 손가락이 감지됨
      // 라이브러리의 checkForBeat 함수 사용 (heartRate.h에 정의됨)
      if (checkForBeat(irValue)) {  // 충돌 문제 없음 - 라이브러리 함수 사용
        long delta = millis() - lastBeat;
        lastBeat = millis();
        
        int beatsPerMinute = 60 / (delta / 1000.0);
        
        if (beatsPerMinute < 255 && beatsPerMinute > 20) {
          rates[rateSpot++] = (byte)beatsPerMinute;
          rateSpot %= RATE_SIZE;
          
          // 평균 계산
          beatAvg = 0;
          for (byte x = 0; x < RATE_SIZE; x++) {
            beatAvg += rates[x];
          }
          beatAvg /= RATE_SIZE;
        }
      }
      
      // 간단한 SpO2 계산 (실제로는 더 복잡한 알고리즘 필요)
      long redValue = particleSensor.getRed();
      if (redValue > 50000 && irValue > 50000) {
        float ratio = (float)redValue / (float)irValue;
        spo2Measured = 110 - 25 * ratio; // 간단한 근사치 계산
        if (spo2Measured > 100) spo2Measured = 100;
        if (spo2Measured < 80) spo2Measured = 80;
      }
      
      Serial.print("심박수: "); Serial.print(beatAvg);
      Serial.print(", SpO2: "); Serial.println(spo2Measured);
    } else {
      // 손가락이 감지되지 않음
      Serial.println("손가락을 센서에 올려주세요");
    }
    
    delay(100);
  }
  
  // 최종 결과 저장 (변수명 변경됨)
  heartRateValue = beatAvg;
  spo2Value = spo2Measured;
  
  Serial.println("측정 완료");
  Serial.print("심박수: "); Serial.print(heartRateValue);
  Serial.print(", SpO2: "); Serial.println(spo2Value);
  
  i2cBusy = false;
}

// 아두이노에 명령 전송
void sendCommand(char command) {
  // 명령 전송 전 충분한 지연 시간 추가
  delay(10);
  
  // 안전하게 I2C 통신 시작
  Wire.beginTransmission(SLAVE_ADDR);
  delay(5);  // 송신 전 지연
  
  // 명령 전송
  Wire.write(command);  // 문자형 명령 전송
  delay(5);  // 명령 전송 후 지연
  
  // 명령 전송 완료
  byte error = Wire.endTransmission();
  delay(10);  // 통신 완료 후 안정화 시간
  
  // 결과 확인
  if (error == 0) {
    Serial.print("명령 전송 성공: ");
    Serial.write(command);
    Serial.println();
  } else {
    Serial.print("명령 전송 실패, 오류 코드: ");
    Serial.print(error);
    // 오류 코드 자세히 출력
    switch(error) {
      case 1: Serial.println(" (데이터가 전송 버퍼보다 길다)"); break;
      case 2: Serial.println(" (슬레이브 주소 전송 NACK - 장치 응답 없음)"); break;
      case 3: Serial.println(" (데이터 전송 NACK)"); break;
      case 4: Serial.println(" (기타 오류)"); break;
      default: Serial.println();
    }
  }
}

// 릴레이 상태 확인
void checkRelayStatus() {
  Wire.requestFrom(SLAVE_ADDR, 1);
  if (Wire.available()) {
    byte status = Wire.read();
    
    // 비트별 상태 확인 (0비트: 팬, 1비트: LED, 2비트: 가습기)
    fanStatus = (status & 0x01) > 0;
    ledStatus = (status & 0x02) > 0;
    humidifierStatus = (status & 0x04) > 0;
    
    Serial.print("릴레이 상태 - 팬: ");
    Serial.print(fanStatus ? "켜짐" : "꺼짐");
    Serial.print(", LED: ");
    Serial.print(ledStatus ? "켜짐" : "꺼짐");
    Serial.print(", 가습기: ");
    Serial.println(humidifierStatus ? "켜짐" : "꺼짐");
  }
}

// 팬 토글 함수
void toggleFan() {
  fanStatus = !fanStatus;
  sendCommand(fanStatus ? FAN_ON : FAN_OFF);
}

// LED 토글 함수
void toggleLed() {
  ledStatus = !ledStatus;
  sendCommand(ledStatus ? LED_ON : LED_OFF);
}

// 가습기 토글 함수
void toggleHumidifier() {
  humidifierStatus = !humidifierStatus;
  sendCommand(humidifierStatus ? HUMIDIFIER_ON : HUMIDIFIER_OFF);
}

// 시리얼 명령 처리
void handleSerialCommands() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    
    // 센서 데이터 읽기 명령 추가
    if (command == "temp") {
      // 온도 데이터 읽기
      readTemperatureHumidity();
    }
    else if (command == "heart") {
      // 심박수 데이터 읽기
      measureHeartRate();
    }
    // 간단한 테스트 명령 추가 (1/0)
    else if (command == "1") {
      // LED 켜기 (간단한 통신 테스트용 1 보내기)
      Wire.beginTransmission(SLAVE_ADDR);
      delay(5);  // 짧은 지연 추가
      Wire.write(1);  // 숫자 1 전송 (아스키가 아닌 바이트 값)
      delay(5);  // 짧은 지연 추가
      byte error = Wire.endTransmission();
      
      // 오류 코드 자세히 출력
      if (error == 0) {
        Serial.println("명령 전송 성공: LED 켜기 (1)");
      } else {
        Serial.print("명령 전송 실패, 오류 코드: ");
        Serial.print(error);
        // 오류 코드 설명
        switch(error) {
          case 1: Serial.println(" (데이터가 전송 버퍼보다 길다)"); break;
          case 2: Serial.println(" (슬레이브 주소 전송 NACK - 장치 응답 없음)"); break;
          case 3: Serial.println(" (데이터 전송 NACK)"); break;
          case 4: Serial.println(" (기타 오류)"); break;
          default: Serial.println();
        }
      }
    }
    else if (command == "0") {
      // LED 끄기 (간단한 통신 테스트용 0 보내기)
      Wire.beginTransmission(SLAVE_ADDR);
      Wire.write(0);  // 숫자 0 전송 (아스키가 아닌 바이트 값)
      byte error = Wire.endTransmission();
      
      if (error == 0) {
        Serial.println("명령 전송 성공: LED 끄기 (0)");
      } else {
        Serial.print("명령 전송 실패, 오류 코드: ");
        Serial.println(error);
      }
    }
    // 릴레이 제어 명령 (1~6)
    else if (command == "2") {
      // 팬 끄기
      sendCommand(FAN_OFF);
      Serial.println("명령: 팬 끄기");
    }
    else if (command == "3") {
      // LED 켜기
      sendCommand(LED_ON);
      Serial.println("명령: LED 켜기");
    }
    else if (command == "4") {
      // LED 끄기
      sendCommand(LED_OFF);
      Serial.println("명령: LED 끄기");
    }
    else if (command == "5") {
      // 가습기 켜기
      sendCommand(HUMIDIFIER_ON);
      Serial.println("명령: 가습기 켜기");
    }
    else if (command == "6") {
      // 가습기 끄기
      sendCommand(HUMIDIFIER_OFF);
      Serial.println("명령: 가습기 끄기");
    }
    else if (command == "s" || command == "status") {
      // 릴레이 상태 확인
      checkRelayStatus();
    }
    else if (command == "scan") {
      // I2C 스캔
      scanI2CDevices();
    }
    else if (command == "help") {
      Serial.println("-------------------------------");
      Serial.println("ESP32 명령어 목록:");
      Serial.println("1: LED 켜기    2: 팬 끄기");
      Serial.println("3: LED 켜기   4: LED 끄기");
      Serial.println("5: 가습기 켜기 6: 가습기 끄기");
      Serial.println("s: 릴레이 상태 확인  scan: I2C 스캔");
      Serial.println("temp: 온습도 센서 데이터 읽기  help: 명령어 목록");
      Serial.println("-------------------------------");
    }
    else {
      Serial.println("잘못된 명령어: " + command);
      Serial.println("도움말: 'help' 입력");
    }
  }
}
