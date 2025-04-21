/*
 * ESP32-WROOM-32와 Arduino R4 간 UART 통신 코드
 * - UART2 사용 (RX: GPIO16, TX: GPIO17)
 * - 명령어 기반 통신 프로토콜 구현
 * - 릴레이 제어 기능 (팬, LED, 가습기)
 * - 센서 데이터 수신 기능
 */

// UART2 정의 (Arduino와 통신)
HardwareSerial ArduinoSerial(2); // UART2 사용

// 핀 정의
const int rxPin = 16;  // GPIO16을 RX로 사용
const int txPin = 17;  // GPIO17을 TX로 사용

// 상태 변수
bool fanStatus = false;
bool ledStatus = false;
bool humidifierStatus = false;
float temperature = 0.0;
float humidity = 0.0;

// 명령어 정의
const char CMD_FAN_ON = 'F';
const char CMD_FAN_OFF = 'f';
const char CMD_LED_ON = 'L';
const char CMD_LED_OFF = 'l';
const char CMD_HUM_ON = 'H';
const char CMD_HUM_OFF = 'h';
const char CMD_GET_STATUS = 'S';
const char CMD_GET_TEMP = 'T';
const char CMD_GET_HUMIDITY = 'U';

// 타이머 변수
unsigned long lastStatusRequest = 0;
const unsigned long STATUS_INTERVAL = 5000;  // 5초마다 상태 확인

void setup() {
  // 디버깅용 시리얼 초기화
  Serial.begin(115200);
  Serial.println("ESP32 UART 통신 시작");
  
  // Arduino와 통신할 UART2 초기화
  ArduinoSerial.begin(9600, SERIAL_8N1, rxPin, txPin);
  
  // 초기화 지연
  delay(1000);
  
  // 초기 상태 요청
  requestStatus();
}

void loop() {
  // Arduino로부터 데이터 수신
  if (ArduinoSerial.available()) {
    String data = ArduinoSerial.readStringUntil('\n');
    processArduinoData(data);
  }
  
  // 시리얼 모니터에서 명령 수신
  if (Serial.available()) {
    char cmd = Serial.read();
    processSerialCommand(cmd);
  }
  
  // 주기적으로 상태 요청
  if (millis() - lastStatusRequest > STATUS_INTERVAL) {
    requestStatus();
    requestTemperature();
    requestHumidity();
    lastStatusRequest = millis();
  }
}

// Arduino로부터 받은 데이터 처리
void processArduinoData(String data) {
  Serial.print("Arduino에서 수신: ");
  Serial.println(data);
  
  // 데이터 형식: "키:값"
  int separatorIndex = data.indexOf(':');
  if (separatorIndex > 0) {
    String key = data.substring(0, separatorIndex);
    String value = data.substring(separatorIndex + 1);
    
    if (key == "FAN") {
      fanStatus = (value == "ON");
      Serial.print("팬 상태 업데이트: ");
      Serial.println(fanStatus ? "켜짐" : "꺼짐");
    }
    else if (key == "LED") {
      ledStatus = (value == "ON");
      Serial.print("LED 상태 업데이트: ");
      Serial.println(ledStatus ? "켜짐" : "꺼짐");
    }
    else if (key == "HUM") {
      humidifierStatus = (value == "ON");
      Serial.print("가습기 상태 업데이트: ");
      Serial.println(humidifierStatus ? "켜짐" : "꺼짐");
    }
    else if (key == "TEMP") {
      temperature = value.toFloat();
      Serial.print("온도 업데이트: ");
      Serial.println(temperature);
    }
    else if (key == "HUM_VAL") {
      humidity = value.toFloat();
      Serial.print("습도 업데이트: ");
      Serial.println(humidity);
    }
  }
}

// 시리얼 모니터에서 받은 명령 처리
void processSerialCommand(char cmd) {
  switch (cmd) {
    case '1':  // 팬 토글
      toggleFan();
      break;
    case '2':  // LED 토글
      toggleLED();
      break;
    case '3':  // 가습기 토글
      toggleHumidifier();
      break;
    case 's':  // 상태 요청
      requestStatus();
      break;
    case 't':  // 온도 요청
      requestTemperature();
      break;
    case 'h':  // 습도 요청
      requestHumidity();
      break;
    case '?':  // 도움말
      printHelp();
      break;
  }
}

// 팬 토글
void toggleFan() {
  if (fanStatus) {
    ArduinoSerial.println(CMD_FAN_OFF);
    Serial.println("팬 끄기 명령 전송");
  } else {
    ArduinoSerial.println(CMD_FAN_ON);
    Serial.println("팬 켜기 명령 전송");
  }
  // 상태는 Arduino의 응답으로 업데이트됨
}

// LED 토글
void toggleLED() {
  if (ledStatus) {
    ArduinoSerial.println(CMD_LED_OFF);
    Serial.println("LED 끄기 명령 전송");
  } else {
    ArduinoSerial.println(CMD_LED_ON);
    Serial.println("LED 켜기 명령 전송");
  }
  // 상태는 Arduino의 응답으로 업데이트됨
}

// 가습기 토글
void toggleHumidifier() {
  if (humidifierStatus) {
    ArduinoSerial.println(CMD_HUM_OFF);
    Serial.println("가습기 끄기 명령 전송");
  } else {
    ArduinoSerial.println(CMD_HUM_ON);
    Serial.println("가습기 켜기 명령 전송");
  }
  // 상태는 Arduino의 응답으로 업데이트됨
}

// 상태 요청
void requestStatus() {
  ArduinoSerial.println(CMD_GET_STATUS);
  Serial.println("상태 요청 명령 전송");
}

// 온도 요청
void requestTemperature() {
  ArduinoSerial.println(CMD_GET_TEMP);
  Serial.println("온도 요청 명령 전송");
}

// 습도 요청
void requestHumidity() {
  ArduinoSerial.println(CMD_GET_HUMIDITY);
  Serial.println("습도 요청 명령 전송");
}

// 도움말 출력
void printHelp() {
  Serial.println("\n==== ESP32 UART 제어 도움말 ====");
  Serial.println("1: 팬 켜기/끄기");
  Serial.println("2: LED 켜기/끄기");
  Serial.println("3: 가습기 켜기/끄기");
  Serial.println("s: 상태 요청");
  Serial.println("t: 온도 요청");
  Serial.println("h: 습도 요청");
  Serial.println("?: 도움말 표시");
  Serial.println("==============================\n");
}
