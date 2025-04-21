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

// 버튼 입력 핀 정의
const int BTN_FAN_PIN = 5;      // D5에 팬 제어 버튼 연결
const int BTN_LED_PIN = 6;      // D6에 LED 제어 버튼 연결
const int BTN_HUMID_PIN = 7;    // D7에 가습기 제어 버튼 연결
const int BTN_AUTO_PIN = 8;     // D8에 자동/수동 모드 전환 버튼 연결

// 센서 핀 정의 (아날로그 입력)
const int TEMP_SENSOR_PIN = A0;  // 온도 센서
const int HUMID_SENSOR_PIN = A1; // 습도 센서
const int DISTANCE_SENSOR_PIN = A2; // 거리 센서 (VL53LOX)

// 릴레이 상태 변수
bool fanStatus = false;
bool ledStatus = false;
bool humidifierStatus = false;

// 버튼 상태 변수
bool btnFanState = HIGH;       // 버튼의 현재 상태 (풀업: HIGH가 기본)
bool btnLedState = HIGH;
bool btnHumidState = HIGH;
bool btnAutoState = HIGH;
bool lastBtnFanState = HIGH;   // 버튼의 이전 상태
bool lastBtnLedState = HIGH;
bool lastBtnHumidState = HIGH;
bool lastBtnAutoState = HIGH;
bool autoModeEnabled = false;  // 자동 모드 활성화 상태

// 버튼 디바운싱 변수
unsigned long lastDebounceTime[4] = {0, 0, 0, 0};  // 각 버튼의 마지막 디바운스 시간
const unsigned long DEBOUNCE_DELAY = 50;           // 디바운스 시간 (50ms)

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
const char CMD_GET_DISTANCE = 'D'; // 거리 센서 데이터 전송 명령어

// 센서 데이터 갱신 주기
unsigned long lastSensorUpdate = 0;
const unsigned long SENSOR_UPDATE_INTERVAL = 2000;  // 2초마다 센서 데이터 갱신

// 명령어 접두사 정의
const char CMD_PREFIX[] = "CMD:";  // 명령어 접두사
const char ACK_PREFIX[] = "ACK:";  // 응답 접두사
const char ERR_PREFIX[] = "ERR:";  // 오류 접두사
const char BTN_PREFIX[] = "BTN:";  // 버튼 접두사
const char MODE_PREFIX[] = "MODE:"; // 모드 접두사

// 디버깅 활성화 여부
bool debugEnabled = true;  // 디버깅 활성화 여부

// 디버깅 로그 함수
void debugLog(String tag, String message) {
  if (debugEnabled) {
    // 현재 시간 출력 (밀리초)
    unsigned long currentTime = millis();
    Serial.print("[");
    Serial.print(currentTime);
    Serial.print("] [");
    Serial.print(tag);
    Serial.print("] ");
    Serial.println(message);
  }
}

// 응답 전송 함수
void sendAck(String command) {
  String ackMsg = String(ACK_PREFIX) + command;
  Serial1.println(ackMsg);
  debugLog("ACK", "Sent: " + ackMsg);
}

// 오류 전송 함수
void sendError(String errorCode, String errorMsg) {
  String errStr = String(ERR_PREFIX) + errorCode + ":" + errorMsg;
  Serial1.println(errStr);
  debugLog("ERR", "Sent: " + errStr);
}

// 상태 전송 함수
void sendStatus(String device, String status) {
  String statusMsg = device + ":" + status;
  Serial1.println(statusMsg);
  debugLog("STATUS", "Sent: " + statusMsg);
}

// 설정 함수
void setup() {
  // 시리얼 통신 설정
  Serial.begin(115200);
  Serial.println("Arduino R4 UART 통신 코드");

  // ESP32와의 UART 통신 설정
  Serial1.begin(115200);  // 9600uc5d0uc11c 115200uc73cub85c uc18dub3c4 ud5a5uc0c1
  
  // 릴레이 핀 설정
  pinMode(FAN_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(HUMID_PIN, OUTPUT);

  // 릴레이 초기화
  digitalWrite(FAN_PIN, LOW);
  digitalWrite(LED_PIN, LOW);1
  digitalWrite(HUMID_PIN, LOW);
  
  // 버튼 핀 설정
  pinMode(BTN_FAN_PIN, INPUT_PULLUP);
  pinMode(BTN_LED_PIN, INPUT_PULLUP);
  pinMode(BTN_HUMID_PIN, INPUT_PULLUP);
  pinMode(BTN_AUTO_PIN, INPUT_PULLUP);
  
  // 센서 핀 설정
  pinMode(TEMP_SENSOR_PIN, INPUT);
  pinMode(HUMID_SENSOR_PIN, INPUT);
  pinMode(DISTANCE_SENSOR_PIN, INPUT);
  
  // 초기화 대기
  delay(1000);
}

// 명령어 처리 함수
void processCommand(String command) {
  // 디버깅 로그 출력
  debugLog("CMD", "ESP32에서 명령어 수신: [" + command + "]");
  
  // 응답 전송
  sendAck(command);
  
  if (command.length() < 1) {
    sendError("CMD_EMPTY", "Empty command");
    return;
  }
  
  // 명령어 접두사 확인
  if (command.startsWith(CMD_PREFIX) && command.length() >= 5) {
    char cmd = command.charAt(4);
    processCommandChar(cmd);
  } 
  else {
    // 명령어 접두사 없이 전송된 명령어 처리
    if (command.length() >= 1) {
      char cmd = command.charAt(0);
      processCommandChar(cmd);
    }
  }
}

// 명령어 처리 함수 (명령어 문자 처리)
void processCommandChar(char cmd) {
  debugLog("EXEC", String("Command char: ") + cmd);
  
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
      sendAllStatus();
      break;
    case CMD_GET_TEMP:
      sendTemperature();
      break;
    case CMD_GET_HUMIDITY:
      sendHumidity();
      break;
    case CMD_GET_DISTANCE:
      sendDistance();
      break;
    default:
      sendError("CMD_UNKNOWN", String("Unknown command: ") + cmd);
      break;
  }
}

// 팬 제어 함수
void setFan(bool state) {
  digitalWrite(FAN_PIN, state ? HIGH : LOW);
  fanStatus = state;
  debugLog("FAN", String("팬 상태: ") + (state ? "ON" : "OFF"));
  sendStatus("FAN", state ? "ON" : "OFF");
}

// LED 제어 함수
void setLED(bool state) {
  digitalWrite(LED_PIN, state ? HIGH : LOW);
  ledStatus = state;
  debugLog("LED", String("LED 상태: ") + (state ? "ON" : "OFF"));
  sendStatus("LED", state ? "ON" : "OFF");
}

// 가습기 제어 함수
void setHumidifier(bool state) {
  digitalWrite(HUMID_PIN, state ? HIGH : LOW);
  humidifierStatus = state;
  debugLog("HUMID", String("가습기 상태: ") + (state ? "ON" : "OFF"));
  sendStatus("HUM", state ? "ON" : "OFF");
}

// 모든 상태 전송 함수
void sendAllStatus() {
  debugLog("STATUS", "모든 상태 전송");
  
  // 팬 상태 전송
  sendStatus("FAN", fanStatus ? "ON" : "OFF");
  delay(20); // 상태 전송 간격
  
  // LED 상태 전송
  sendStatus("LED", ledStatus ? "ON" : "OFF");
  delay(20);
  
  // 가습기 상태 전송
  sendStatus("HUM", humidifierStatus ? "ON" : "OFF");
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

// 거리 전송 함수
void sendDistance() {
  float distance = readDistance();
  Serial.print("거리: ");
  Serial.print(distance);
  Serial.println(" mm");
  
  Serial1.print("DISTANCE:");
  Serial1.println(distance);
}

// 센서 데이터 갱신 및 전송
void updateAndSendSensorData() {
  sendTemperature();
  delay(50);
  sendHumidity();
  delay(50);
  sendDistance();
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
  int sensorValue = analogRead(HUMID_SENSOR_PIN); // 실제 센서가 연결되면 아래 코드를 사용
  float humidity = map(sensorValue, 0, 1023, 20, 90); // 실제 센서가 연결되면 아래 코드를 사용
  humidity += random(-3, 4); // 랜덤 오차 추가
  return humidity;
}

// 거리 읽기 (테스트용 더미 값 생성)
float readDistance() {
  // 실제 센서가 연결되면 아래 코드를 사용
  // int rawValue = analogRead(DISTANCE_SENSOR_PIN);
  // float distance = map(rawValue, 0, 1023, 50, 1200);
  
  // 테스트용 더미 값
  int sensorValue = analogRead(DISTANCE_SENSOR_PIN); // 실제 센서가 연결되면 아래 코드를 사용
  float distance = map(sensorValue, 0, 1023, 50, 1200); // 실제 센서가 연결되면 아래 코드를 사용
  distance += random(-10, 11); // 랜덤 오차 추가
  return distance;
}

// 버튼 입력 처리 함수
void processButtonInput() {
  // 버튼 상태 읽기
  btnFanState = digitalRead(BTN_FAN_PIN);
  btnLedState = digitalRead(BTN_LED_PIN);
  btnHumidState = digitalRead(BTN_HUMID_PIN);
  btnAutoState = digitalRead(BTN_AUTO_PIN);
  
  // 버튼 디버깅
  static unsigned long lastButtonDebugTime = 0;
  if (millis() - lastButtonDebugTime > 1000) { // 1초마다 버튼 디버깅
    lastButtonDebugTime = millis();
    Serial.print("\n버튼 상태 - 팬:");
    Serial.print(btnFanState ? "HIGH" : "LOW");
    Serial.print(", LED:");
    Serial.print(btnLedState ? "HIGH" : "LOW");
    Serial.print(", 가습기:");
    Serial.print(btnHumidState ? "HIGH" : "LOW");
    Serial.print(", 자동모드:");
    Serial.println(btnAutoState ? "HIGH" : "LOW");
  }
  
  // 버튼 디바운싱
  unsigned long currentTime = millis();
  if (btnFanState != lastBtnFanState) {
    if (currentTime - lastDebounceTime[0] > DEBOUNCE_DELAY) {
      lastDebounceTime[0] = currentTime;
      if (btnFanState == LOW) {
        // 팬 버튼 눌림
        setFan(!fanStatus);
      }
    }
  }
  if (btnLedState != lastBtnLedState) {
    if (currentTime - lastDebounceTime[1] > DEBOUNCE_DELAY) {
      lastDebounceTime[1] = currentTime;
      if (btnLedState == LOW) {
        // LED 버튼 눌림
        setLED(!ledStatus);
      }
    }
  }
  if (btnHumidState != lastBtnHumidState) {
    if (currentTime - lastDebounceTime[2] > DEBOUNCE_DELAY) {
      lastDebounceTime[2] = currentTime;
      if (btnHumidState == LOW) {
        // 가습기 버튼 눌림
        setHumidifier(!humidifierStatus);
      }
    }
  }
  if (btnAutoState != lastBtnAutoState) {
    if (currentTime - lastDebounceTime[3] > DEBOUNCE_DELAY) {
      lastDebounceTime[3] = currentTime;
      if (btnAutoState == LOW) {
        // 자동 모드 버튼 눌림
        autoModeEnabled = !autoModeEnabled;
        Serial.println(String("자동 모드 ") + (autoModeEnabled ? "활성화" : "비활성화"));
      }
    }
  }
  
  // 이전 버튼 상태 저장
  lastBtnFanState = btnFanState;
  lastBtnLedState = btnLedState;
  lastBtnHumidState = btnHumidState;
  lastBtnAutoState = btnAutoState;
}

// 메인 루프 함수
void loop() {
  // ESP32에서 명령어 수신
  if (Serial1.available()) {
    String command = Serial1.readStringUntil('\n');
    command.trim();  // 앞뒤 공백 제거
    if (command.length() > 0) {
      processCommand(command);
    }
  }
  
  // 버튼 입력 처리
  processButtonInput();
  
  // 센서 데이터 갱신 및 전송
  if (millis() - lastSensorUpdate > SENSOR_UPDATE_INTERVAL) {
    updateAndSendSensorData();
    lastSensorUpdate = millis();
  }
}
