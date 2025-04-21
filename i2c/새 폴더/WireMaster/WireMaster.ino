#include "Wire.h"

#define I2C_DEV_ADDR 0x13

uint32_t i = 0;

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Wire.begin();
  
  // I2C 통신 속도를 50kHz로 설정 (아두이노와의 호환성 향상)
  Wire.setClock(100000);
  
  Serial.println("ESP32 I2C 마스터 초기화 완료");
  Serial.print("슬레이브 주소: 0x");
  Serial.println(I2C_DEV_ADDR, HEX);
  
  // 초기화 지연시간 추가 (아두이노 슬레이브가 준비될 시간 제공)
  delay(200);
}

void loop() {
  Serial.println("\n--------- 통신 시작 ---------");
  delay(5000);

  //슬레이브에 메시지 전송
  Serial.println("슬레이브에 데이터 전송 중...");
  Wire.beginTransmission(I2C_DEV_ADDR);
  Wire.printf("Hello World! %lu", i++);
  uint8_t error = Wire.endTransmission(true);
  Serial.printf("전송 결과: %u (0=성공, 1-4=오류)\n", error);

  //슬레이브로부터 16바이트 읽기
  Serial.println("슬레이브에서 데이터 요청 중...");
  uint8_t bytesReceived = Wire.requestFrom(I2C_DEV_ADDR, 16);
  Serial.printf("수신된 바이트 수: %u\n", bytesReceived);
  if ((bool)bytesReceived) {  //0보다 많은 바이트를 수신한 경우
    uint8_t temp[bytesReceived];
    Wire.readBytes(temp, bytesReceived);
    log_print_buf(temp, bytesReceived);
  } else {
    Serial.println("데이터 수신 실패");
  }
}