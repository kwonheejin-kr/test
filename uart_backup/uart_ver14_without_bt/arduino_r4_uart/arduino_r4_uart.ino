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

// UART 통신 변수
String rxBuffer = "";  // UART 수신 데이터 버퍼

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
  digitalWrite(LED_PIN, LOW);
  digitalWrite(HUMID_PIN, LOW);
  
  // 버튼 핀 설정
  pinMode(BTN_FAN_PIN, INPUT_PULLUP);
  pinMode(BTN_LED_PIN, INPUT_PULLUP);
  pinMode(BTN_HUMID_PIN, INPUT_PULLUP);
  pinMode(BTN_AUTO_PIN, INPUT_PULLUP);
  
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
      sendAck("FAN_ON");
      break;
    case CMD_FAN_OFF:
      setFan(false);
      sendAck("FAN_OFF");
      break;
    case CMD_LED_ON:
      setLED(true);
      sendAck("LED_ON");
      break;
    case CMD_LED_OFF:
      setLED(false);
      sendAck("LED_OFF");
      break;
    case CMD_HUM_ON:
      setHumidifier(true);
      sendAck("HUM_ON");
      break;
    case CMD_HUM_OFF:
      setHumidifier(false);
      sendAck("HUM_OFF");
      break;
    case CMD_GET_STATUS:
      sendAllStatus();
      break;
    case CMD_GET_TEMP:
    case CMD_GET_HUMIDITY:
    case CMD_GET_DISTANCE:
      // ESP32로부터 센서 데이터 요청이 왔지만, 아두이노는 센서가 없음
      // 따라서 오류 메시지 반환
      sendError("SENSOR_ERR", "No sensors connected to Arduino");
      break;
    default:
      sendError("UNKNOWN_CMD", "Unknown command: " + String(cmd));
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
  // 릴레이 상태 전송
  sendStatus("FAN", fanStatus ? "ON" : "OFF");
  sendStatus("LED", ledStatus ? "ON" : "OFF");
  sendStatus("HUM", humidifierStatus ? "ON" : "OFF");
  sendStatus("AUTO", autoModeEnabled ? "ON" : "OFF");
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
        
        // ESP32에 자동 모드 상태 전송
        Serial1.print(MODE_PREFIX);
        Serial1.println(autoModeEnabled ? "AUTO" : "MANUAL");
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
  // 버튼 입력 처리
  processButtonInput();
  
  // UART 수신 데이터 처리
  while (Serial1.available() > 0) {
    char inChar = (char)Serial1.read();
    if (inChar == '\n') {
      // 명령어 처리
      processCommand(rxBuffer);
      rxBuffer = ""; // 버퍼 초기화
    } else {
      rxBuffer += inChar; // 버퍼에 문자 추가
    }
  }
}
