#include <Wire.h>

// 릴레이 핀 정의
const int FAN_PIN = 2;
const int LED_PIN = 3;
const int SMOG_PIN = 4;

// I2C 슬레이브 주소
const int SLAVE_ADDRESS = 0x08;

// 현재 상태 저장
bool fanState = false;
bool ledState = false;
bool smogState = false;

void setup() {
    // 시리얼 초기화
    Serial.begin(115200);
    
    // 릴레이 핀 초기화
    pinMode(FAN_PIN, OUTPUT);
    pinMode(LED_PIN, OUTPUT);
    pinMode(SMOG_PIN, OUTPUT);
    
    // 초기 상태 설정
    digitalWrite(FAN_PIN, LOW);
    digitalWrite(LED_PIN, LOW);
    digitalWrite(SMOG_PIN, LOW);
    
    // I2C 슬레이브 초기화
    Wire.begin(SLAVE_ADDRESS);
    Wire.onReceive(receiveEvent);
    Wire.onRequest(requestEvent);
    
    Serial.println("Arduino R4 WiFi 초기화 완료");
}

void loop() {
    // 필요한 경우 여기에 추가 로직 구현
    delay(100);
}

// I2C 데이터 수신 처리
void receiveEvent(int howMany) {
    if (Wire.available()) {
        char command = Wire.read();
        
        switch (command) {
            case 'T':  // 팬 토글
                fanState = !fanState;
                digitalWrite(FAN_PIN, fanState ? HIGH : LOW);
                Serial.print("팬 상태 변경: ");
                Serial.println(fanState ? "ON" : "OFF");
                break;
                
            case 'L':  // LED 토글
                ledState = !ledState;
                digitalWrite(LED_PIN, ledState ? HIGH : LOW);
                Serial.print("LED 상태 변경: ");
                Serial.println(ledState ? "ON" : "OFF");
                break;
                
            case 'H':  // 가습기 토글
                smogState = !smogState;
                digitalWrite(SMOG_PIN, smogState ? HIGH : LOW);
                Serial.print("가습기 상태 변경: ");
                Serial.println(smogState ? "ON" : "OFF");
                break;
                
            case 'S':  // 상태 요청
                // 처리 없음 - requestEvent에서 처리
                break;
        }
    }
}

// I2C 상태 요청 응답
void requestEvent() {
    // 현재 상태 전송 (3바이트: 팬, LED, 가습기)
    Wire.write(fanState ? '1' : '0');
    Wire.write(ledState ? '1' : '0');
    Wire.write(smogState ? '1' : '0');
}