#include <Wire.h>

#define I2C_DEV_ADDR 0x13

uint32_t i = 0;
char buffer[64]; // 수신 버퍼

// 마스터의 데이터 요청 시 호출되는 함수
void onRequest() {
  // 간단한 응답 메시지 생성
  char response[32];
  sprintf(response, "%lu Packets", i++);
  
  // 응답 전송 (Arduino 방식)
  Wire.write(response, strlen(response));
  
  Serial.print("응답 전송: ");
  Serial.println(response);
}

// 마스터로부터 데이터 수신 시 호출되는 함수
void onReceive(int len) {
  int i = 0;
  Serial.print("데이터 수신[");
  Serial.print(len);
  Serial.print("바이트]: ");
  
  // 수신된 모든 바이트 읽기
  while(Wire.available() && i < 63) {
    buffer[i++] = Wire.read();
  }
  buffer[i] = '\0'; // 문자열 종료
  
  Serial.println(buffer);
}

void setup() {
  Serial.begin(115200);
  Serial.println("I2C 슬레이브 초기화 중...");
  
  // 중요: 먼저 Wire 라이브러리 초기화
  Wire.begin(I2C_DEV_ADDR);
  
  // 그 다음 콜백 등록
  Wire.onReceive(onReceive);
  Wire.onRequest(onRequest);
  
  Serial.print("슬레이브 주소: 0x");
  Serial.println(I2C_DEV_ADDR, HEX);
  Serial.println("I2C 슬레이브 준비 완료!");
}

void loop() {
  // 특별한 작업 없음, 인터럽트 기반으로 작동
  delay(5000);
  Serial.println("슬레이브 대기 중...");
}