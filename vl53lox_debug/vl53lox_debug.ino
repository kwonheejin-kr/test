/*
 * VL53L0X 센서 디버그 프로그램
 * ESP32-WROOM-32 최적화 버전
 * 버전: 1.0.0 (2025-03-31)
 */

#include <Wire.h>
#include "Adafruit_VL53L0X.h"

// ESP32 I2C 핀 설정
#define SDA_PIN 21
#define SCL_PIN 22

// XSHUT 핀 설정 (선택사항)
#define XSHUT_PIN 13

// 시리얼 통신 속도
#define SERIAL_BAUD 115200

// VL53L0X 센서 객체 생성
Adafruit_VL53L0X lox = Adafruit_VL53L0X();

void setup() {
  // 시리얼 통신 시작
  Serial.begin(SERIAL_BAUD);
  delay(100); // 시리얼 포트 준비 시간
  
  Serial.println("\n=== VL53L0X 센서 디버그 테스트 시작 ===");
  
  // I2C 핀 설정 (ESP32용)
  Wire.begin(SDA_PIN, SCL_PIN);
  
  // I2C 통신 속도 설정 (ESP32-WROOM-32의 안정성을 위해 낮은 속도 사용)
  Wire.setClock(100000); // 100kHz
  
  // XSHUT 핀 사용 시 설정
  #ifdef XSHUT_PIN
    Serial.println("XSHUT 핀을 사용한 하드웨어 리셋 수행");
    pinMode(XSHUT_PIN, OUTPUT);
    digitalWrite(XSHUT_PIN, LOW);  // 센서 비활성화
    delay(10);
    digitalWrite(XSHUT_PIN, HIGH); // 센서 활성화
    delay(10);
  #endif
  
  // I2C 장치 스캔
  scanI2CDevices();
  
  // VL53L0X 센서 초기화
  Serial.println("\nVL53L0X 센서 초기화 시도 중...");
  
  // 초기화 재시도 로직
  bool initSuccess = false;
  for (int attempt = 1; attempt <= 3; attempt++) {
    Serial.print("시도 #"); Serial.print(attempt); Serial.println(":");
    
    // begin() 함수 원형: boolean begin(uint8_t i2c_addr = VL53L0X_I2C_ADDR, boolean debug = false, TwoWire *i2c = &Wire)
    initSuccess = lox.begin(0x29, false, &Wire);
    
    if (initSuccess) {
      Serial.println("VL53L0X 센서 초기화 성공!");
      break;
    } else {
      Serial.println("VL53L0X 센서 초기화 실패");
      delay(100);
    }
  }
  
  if (!initSuccess) {
    Serial.println("\n[오류] VL53L0X 센서를 초기화할 수 없습니다.");
    Serial.println("다음 사항을 확인하세요:");
    Serial.println("1. 센서가 올바르게 연결되어 있는지");
    Serial.println("2. I2C 주소가 올바른지 (기본: 0x29)");
    Serial.println("3. 전원이 안정적인지 (VIN: 3.3V)");
    Serial.println("4. 배선이 올바른지 (SDA: GPIO21, SCL: GPIO22)");
    Serial.println("센서 초기화에 실패했으나 계속 진행합니다...");
  } else {
    // 고급 설정
    Serial.println("\nVL53L0X 고급 설정 구성 중...");
    
    // 측정 타이밍 버짓 설정 (기본값: 33000 마이크로초 = 33ms)
    Serial.print("측정 타이밍 버짓 설정: ");
    bool timingResult = lox.setMeasurementTimingBudgetMicroSeconds(50000); // 50ms
    Serial.println(timingResult ? "성공" : "실패");
    
    // VCSEL 펄스 기간 설정 (선택사항)
    Serial.print("VCSEL 프리레인지 펄스 기간 설정: ");
    bool vcsel1Result = lox.setVcselPulsePeriod(VL53L0X_VCSEL_PERIOD_PRE_RANGE, 18);
    Serial.println(vcsel1Result ? "성공" : "실패");
    
    Serial.print("VCSEL 파이널레인지 펄스 기간 설정: ");
    bool vcsel2Result = lox.setVcselPulsePeriod(VL53L0X_VCSEL_PERIOD_FINAL_RANGE, 14);
    Serial.println(vcsel2Result ? "성공" : "실패");
  }
  
  Serial.println("\n측정을 시작합니다...");
  Serial.println("==========================");
}

void loop() {
  VL53L0X_RangingMeasurementData_t measure;
  
  // 측정 수행
  lox.rangingTest(&measure, false); // false: 디버그 정보 출력 안 함
  
  // 측정 결과 출력
  Serial.print("상태: ");
  switch (measure.RangeStatus) {
    case 0: Serial.print("유효한 측정"); break;
    case 1: Serial.print("신호 약함"); break;
    case 2: Serial.print("신호 너무 강함"); break;
    case 3: Serial.print("위상 오류"); break;
    case 4: Serial.print("하드웨어 오류"); break;
    case 5: Serial.print("타임아웃"); break;
    case 6: Serial.print("신호 간섭"); break;
    case 7: Serial.print("임계치 초과"); break;
    default: Serial.print("알 수 없는 오류"); break;
  }
  Serial.print(" (");
  Serial.print(measure.RangeStatus);
  Serial.print(") | ");
  
  // 거리 출력 (상태 코드가 4가 아닌 경우만 유효)
  if (measure.RangeStatus != 4) {
    Serial.print("거리: ");
    Serial.print(measure.RangeMilliMeter);
    Serial.println(" mm");
  } else {
    Serial.println("측정 범위 초과");
  }
  
  // 1초 대기
  delay(1000);
}

// I2C 장치 스캔 함수
void scanI2CDevices() {
  byte error, address;
  int deviceCount = 0;
  
  Serial.println("I2C 장치 스캔 중...");
  
  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    
    if (error == 0) {
      Serial.print("I2C 장치 발견! 주소: 0x");
      if (address < 16) {
        Serial.print("0");
      }
      Serial.print(address, HEX);
      
      // VL53L0X 기본 주소인 경우 표시
      if (address == 0x29) {
        Serial.print(" (VL53L0X 기본 주소)");
      }
      
      Serial.println();
      deviceCount++;
    }
  }
  
  if (deviceCount == 0) {
    Serial.println("I2C 장치를 찾을 수 없습니다.");
  } else {
    Serial.print("총 ");
    Serial.print(deviceCount);
    Serial.println("개의 I2C 장치를 찾았습니다.");
  }
}
