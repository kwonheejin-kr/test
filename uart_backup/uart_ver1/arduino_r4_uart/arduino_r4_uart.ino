/*
 * Arduino R4와 ESP32-WROOM-32 간 UART 통신 코드
 * - Serial1 사용 (핀 0, 1)
 * - 명령어 기반 통신 프로토콜 구현
 * - 릴레이 제어 기능 (팬, LED, 가습기)
 * - 센서 데이터 전송 기능
 */

// 릴레이 제어 핀 정의
const int FAN_PIN = 2;      // D2에 팬 릴레이 연결
const int LED_PIN = 3;      // D3에 LED 릴레이 연결
const int HUMID_PIN = 4;    // D4에 가습기 릴레이 연결

// 센서 핀 정의 (아날로그 입력)
const int TEMP_SENSOR_PIN = A0;  // 온도 센서
const int HUMID_SENSOR_PIN = A1; // 습도 센서

// 릴레이 상태 변수
bool fanStatus = false;
bool ledStatus = false;
bool humidifierStatus = false;

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

// 센서 데이터 갱신 주기
unsigned long lastSensorUpdate = 0;
const unsigned long SENSOR_UPDATE_INTERVAL = 2000;  // 2초마다 센서 데이터 갱신

// 설정 함수
void setup() {
  // 시리얼 통신 설정
  Serial.begin(9600);
  Serial.println("Arduino R4 UART 통신 코드");

  // ESP32와의 UART 통신 설정
  Serial1.begin(9600);
  
  // 릴레이 핀 설정
  pinMode(FAN_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(HUMID_PIN, OUTPUT);

  // 릴레이 초기화
  digitalWrite(FAN_PIN, LOW);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(HUMID_PIN, LOW);
  
  // 센서 핀 설정
  pinMode(TEMP_SENSOR_PIN, INPUT);
  pinMode(HUMID_SENSOR_PIN, INPUT);
  
  // 초기화 대기
  delay(1000);
}

// 메인 루프 함수
void loop() {
  // ESP32에서 명령어 수신
  if (Serial1.available()) {
    String command = Serial1.readStringUntil('\n');
    processCommand(command);
  }
  
  // 센서 데이터 갱신 및 전송
  if (millis() - lastSensorUpdate > SENSOR_UPDATE_INTERVAL) {
    updateAndSendSensorData();
    lastSensorUpdate = millis();
  }
}

// 명령어 처리 함수
void processCommand(String command) {
  Serial.print("ESP32에서 명령어 수신: ");
  Serial.println(command);
  
  if (command.length() < 1) {
    return;
  }
  
  char cmd = command.charAt(0);
  
  switch (cmd) {
    case CMD_FAN_ON:
      setFan(true);
      break;
    case CMD_FAN_OFF:
      setFan(false);
      break;
    case CMD_LED_ON:
      setLED(true);
      break;
    case CMD_LED_OFF:
      setLED(false);
      break;
    case CMD_HUM_ON:
      setHumidifier(true);
      break;
    case CMD_HUM_OFF:
      setHumidifier(false);
      break;
    case CMD_GET_STATUS:
      sendStatus();
      break;
    case CMD_GET_TEMP:
      sendTemperature();
      break;
    case CMD_GET_HUMIDITY:
      sendHumidity();
      break;
  }
}

// 팬 제어 함수
void setFan(bool state) {
  digitalWrite(FAN_PIN, state ? HIGH : LOW);
  fanStatus = state;
  Serial.print("팬 상태: ");
  Serial.println(state ? "ON" : "OFF");
  Serial1.print("FAN:");
  Serial1.println(state ? "ON" : "OFF");
}

// LED 제어 함수
void setLED(bool state) {
  digitalWrite(LED_PIN, state ? HIGH : LOW);
  ledStatus = state;
  Serial.print("LED 상태: ");
  Serial.println(state ? "ON" : "OFF");
  Serial1.print("LED:");
  Serial1.println(state ? "ON" : "OFF");
}

// 가습기 제어 함수
void setHumidifier(bool state) {
  digitalWrite(HUMID_PIN, state ? HIGH : LOW);
  humidifierStatus = state;
  Serial.print("가습기 상태: ");
  Serial.println(state ? "ON" : "OFF");
  Serial1.print("HUM:");
  Serial1.println(state ? "ON" : "OFF");
}

// 릴레이 상태 전송 함수
void sendStatus() {
  Serial.println("릴레이 상태 전송");
  // 팬 상태 전송
  Serial1.print("FAN:");
  Serial1.println(fanStatus ? "ON" : "OFF");
  delay(50);
  
  // LED 상태 전송
  Serial1.print("LED:");
  Serial1.println(ledStatus ? "ON" : "OFF");
  delay(50);
  
  // 가습기 상태 전송
  Serial1.print("HUM:");
  Serial1.println(humidifierStatus ? "ON" : "OFF");
}

// 온도 전송 함수
void sendTemperature() {
  float temperature = readTemperature();
  Serial.print("온도: ");
  Serial.println(temperature);
  
  Serial1.print("TEMP:");
  Serial1.println(temperature);
}

// 습도 전송 함수
void sendHumidity() {
  float humidity = readHumidity();
  Serial.print("습도: ");
  Serial.println(humidity);
  
  Serial1.print("HUM_VAL:");
  Serial1.println(humidity);
}

// 센서 데이터 갱신 및 전송
void updateAndSendSensorData() {
  // 온도 및 습도 전송
  sendTemperature();
  delay(50);
  sendHumidity();
}

// 온도 읽기 (테스트용 더미 값 생성)
float readTemperature() {
  // 실제 센서가 연결되면 아래 코드를 사용
  // int rawValue = analogRead(TEMP_SENSOR_PIN);
  // float voltage = rawValue * (3.3 / 1023.0);
  // float temperature = (voltage - 0.5) * 100;
  
  // 테스트용 더미 값
  return 23.5 + (random(-10, 11) / 10.0); // 22.5에서 24.5 사이의 랜덤 값
}

// 습도 읽기 (테스트용 더미 값 생성)
float readHumidity() {
  // 실제 센서가 연결되면 아래 코드를 사용
  // int rawValue = analogRead(HUMID_SENSOR_PIN);
  // float humidity = map(rawValue, 0, 1023, 0, 100);
  
  // 테스트용 더미 값
  return 55.0 + (random(-50, 51) / 10.0); // 50.0에서 60.0 사이의 랜덤 값
}
