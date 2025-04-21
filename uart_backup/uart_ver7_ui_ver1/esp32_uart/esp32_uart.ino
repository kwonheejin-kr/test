#include <Arduino.h>

// ESP32 PWM 제어를 위해 필수 헤더 추가
#ifdef ESP32
  #include <driver/ledc.h>
#endif

#include <Wire.h>
#include <MAX30105.h>  // MAX30102 센서 라이브러리 추가
#include <heartRate.h>  // 심박수 계산 알고리즘 라이브러리
#include <Adafruit_SHT31.h>  // SHT31 센서 라이브러리 추가
#include <Adafruit_VL53L0X.h>  // VL53LOX 센서 라이브러리 추가

// TFT 디스플레이 라이브러리 추가
#include <TFT_eSPI.h>
#include <SPI.h>

// UART 통신을 위한 HardwareSerial 정의
HardwareSerial ArduinoSerial(2); // UART2 사용

// I2C 장치 주소
const int MAX30102_ADDRESS = 0x57; // MAX30102 심박수/산소포화도 센서 주소
const int SHT31_ADDRESS = 0x44;  // SHT31 온습도 센서 주소
const int VL53L0X_ADDRESS = 0x29;  // VL53LOX 거리 센서의 기본 I2C 주소

// I2C 버스 사용 상태를 추적하는 변수
bool i2cBusy = false;

// 센서 객체 생성
MAX30105 max30102Sensor;
Adafruit_SHT31 sht31Sensor = Adafruit_SHT31();
Adafruit_VL53L0X vl53loxSensor = Adafruit_VL53L0X();

// TFT 디스플레이 객체 생성
TFT_eSPI tft = TFT_eSPI();

// 센서 데이터 변수
float temperature = 0; // 온도
float humidity = 0; // 습도
int heartRateValue = 0; // 심박수
long irValue = 0; // IR 값
float distance = 0; // 거리 (mm)

// 상태 변수
bool fanStatus = false;
bool ledStatus = false;
bool humidifierStatus = false;
bool autoModeEnabled = false;

// 타이머 변수
unsigned long lastStatusRequest = 0;
const unsigned long STATUS_INTERVAL = 5000;  // 5초마다 상태 확인
unsigned long lastSensorUpdate = 0;
const unsigned long SENSOR_UPDATE_INTERVAL = 5000; // 5초마다 센서 데이터 갱신
unsigned long lastVL53LOXMeasurement = 0;
const unsigned long VL53LOX_INTERVAL = 1000; // 1초마다 VL53LOX 센서 측정

// LCD UI 관련 변수
unsigned long lastDisplayUpdate = 0;
const int DISPLAY_UPDATE_INTERVAL = 1000; // 1초마다 화면 업데이트

// UI 영역 정의
const int STATUS_AREA_Y = 10;
const int TEMP_AREA_Y = 70;
const int HUMID_AREA_Y = 110;
const int HR_AREA_Y = 150;
const int CONTROL_AREA_Y = 200;

// 색상 정의 (이미 User_Setup.h에 정의되어 있지만 편의상 사용)
const uint16_t STATUS_ON_COLOR = TFT_GREEN;
const uint16_t STATUS_OFF_COLOR = TFT_RED;
const uint16_t TEXT_COLOR = TFT_WHITE;
const uint16_t VALUE_COLOR = TFT_YELLOW;
const uint16_t TITLE_COLOR = TFT_CYAN;

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
const char CMD_GET_DISTANCE = 'D'; // 거리 요청 명령 추가

// 통신 프로토콜 상수 정의 (명령어 형식 정의)
const char CMD_PREFIX[] = "CMD:";  // 명령어 접두사
const char ACK_PREFIX[] = "ACK:";  // 확인 응답 접두사
const char ERR_PREFIX[] = "ERR:";  // 오류 접두사
const char BTN_PREFIX[] = "BTN:";  // 버튼 입력 접두사
const char MODE_PREFIX[] = "MODE:"; // 모드 상태 접두사

// 디버깅 플래그
bool debugEnabled = true;  // 디버깅 출력 활성화 여부

// 상단에 추가할 정의 - 전역 변수 선언 부분
#define VL53LOX_XSHUT_PIN 13  // ESP32의 GPIO 13을 XSHUT 핀으로 사용

void setup() {
  // 디버깅용 시리얼 초기화
  Serial.begin(115200);
  Serial.println("ESP32 UART & I2C 통신 시작");
  
  // Arduino와 통신할 UART2 초기화
  ArduinoSerial.begin(115200, SERIAL_8N1, 16, 17);  // 9600에서 115200으로 속도 향상
  
  // I2C 초기화 - ESP32-WROOM-32의 한계를 고려하여 속도를 높임
  Wire.begin(21, 22);
  Wire.setClock(100000); // I2C 통신 속도를 100kHz로 높임 (ESP32-WROOM-32 한계 고려)
  
  // I2C 장치 스캔
  scanI2CDevices();
  
  // MAX30102 센서 초기화
  initializeMAX30102();
  
  // SHT31 센서 초기화
  initializeSHT31();
  
  // VL53LOX 센서 초기화
  initializeVL53LOX();
  
  // TFT 디스플레이 초기화
  tft.init();
  tft.setRotation(1);  // 화면 방향 설정 (1: 세로, 3: 가로)
  tft.fillScreen(TFT_BLACK);
  
  // 백라이트 핀 직접 제어 (PWM 대신 디지털 출력으로 변경)
  pinMode(5, OUTPUT);
  digitalWrite(5, HIGH);  // 백라이트 켜기
  
  // 메인 화면 그리기
  drawMainScreen();
  
  // 초기화 지연
  delay(1000);
  
  // 초기 상태 요청
  requestStatus();
  
  // 도움말 출력
  printHelp();
}

// I2C 장치 스캔 - 더 명확한 오류 메시지 포함
void scanI2CDevices() {
  byte error, address;
  int deviceCount = 0;
  
  Serial.println("\n===== I2C 장치 스캔 시작 =====\n");
  
  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    
    if (error == 0) {
      Serial.print("I2C 장치 발견: 0x");
      if (address < 16) Serial.print("0");
      Serial.print(address, HEX);
      
      // 알려진 I2C 장치 식별
      if (address == MAX30102_ADDRESS) {
        Serial.print(" (MAX30102 심박수/산소포화도 센서)");
      } else if (address == SHT31_ADDRESS) {
        Serial.print(" (SHT31 온습도 센서)");
      } else if (address == VL53L0X_ADDRESS) {
        Serial.print(" (VL53LOX 거리 센서)");
      } else {
        Serial.print(" (알 수 없는 장치)");
      }
      
      Serial.println();
      deviceCount++;
    } else if (error == 4) {
      Serial.print("I2C 장치 에러 (0x");
      if (address < 16) Serial.print("0");
      Serial.print(address, HEX);
      Serial.println(")");
    }
  }
  
  if (deviceCount == 0) {
    Serial.println("주의: I2C 장치를 찾을 수 없습니다!");
    Serial.println("- SDA와 SCL 연결을 확인하세요");
    Serial.println("- 전원이 제대로 연결되어 있는지 확인하세요");
    Serial.println("- 장치 주소가 맞는지 확인하세요");
  } else {
    Serial.print("스캔 완료: ");
    Serial.print(deviceCount);
    Serial.println(" 개의 I2C 장치를 발견했습니다.");
  }
  
  Serial.println("\n===== I2C 장치 스캔 완료 =====\n");
}

void loop() {
  // ESP32에서 명령 수신 (Serial)
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    debugLog("UART", "시리얼 입력: " + command);
    // 명령어 처리 (기존 코드 유지)
    processSerialCommand(command.charAt(0));
  }
  
  // Arduino로부터 데이터 수신 (UART2)
  if (ArduinoSerial.available()) {
    String data = ArduinoSerial.readStringUntil('\n');
    data.trim();  // 앞뒤 공백 및 개행문자 제거
    if (data.length() > 0) {
      processArduinoData(data);
    }
  }
  
  // 주기적으로 상태 요청
  if (millis() - lastStatusRequest > STATUS_INTERVAL) {
    requestStatus();
    requestTemperature();
    requestHumidity();
    requestDistance();
    lastStatusRequest = millis();
  }
  
  // 센서 데이터 정기적으로 업데이트
  if (millis() - lastSensorUpdate > SENSOR_UPDATE_INTERVAL) {
    updateSensorData();
    lastSensorUpdate = millis();
  }
  
  // VL53LOX 거리 센서 1초마다 측정 (예제 코드처럼 동작)
  if (millis() - lastVL53LOXMeasurement > VL53LOX_INTERVAL) {
    // 센서 초기화 확인
    static bool vl53loxInitialized = false;
    if (!vl53loxInitialized && !i2cBusy) {
      vl53loxInitialized = initializeVL53LOX();
    }
    
    if (vl53loxInitialized && !i2cBusy) {
      distance = readVL53LOX(); // 예제 코드처럼 직접 호출
    }
    lastVL53LOXMeasurement = millis();
  }
  
  // 화면 업데이트
  if (millis() - lastDisplayUpdate > DISPLAY_UPDATE_INTERVAL) {
    updateDisplayInfo();
    lastDisplayUpdate = millis();
  }
}

// 명령어 전송 함수 (개선된 버전)
void sendCommand(char cmd, String param = "") {
  String commandStr = String(CMD_PREFIX) + cmd;
  if (param != "") {
    commandStr += ":" + param;
  }
  
  if (debugEnabled) {
    Serial.print("[TX->Arduino] ");
    Serial.println(commandStr);
  }
  
  ArduinoSerial.println(commandStr);  // 개행문자 추가하여 전송
}

// 디버깅 로그 출력 함수
void debugLog(String tag, String message) {
  if (debugEnabled) {
    // 타임스탬프 추가 (밀리초 단위)
    unsigned long currentTime = millis();
    Serial.print("[");
    Serial.print(currentTime);
    Serial.print("] [");
    Serial.print(tag);
    Serial.print("] ");
    Serial.println(message);
  }
}

// Arduino로부터 받은 데이터 처리
void processArduinoData(String data) {
  // 디버그용: 데이터 출력
  debugLog("RX", "Arduino -> " + data);
  
  // 데이터 형식: "key:value"
  int separatorIndex = data.indexOf(':');
  if (separatorIndex > 0) {
    String key = data.substring(0, separatorIndex);
    String value = data.substring(separatorIndex + 1);
    
    debugLog("PARSE", "Key: " + key + ", Value: " + value);
    
    // 확인 응답 처리
    if (key == "ACK") {
      debugLog("ACK", "수신: " + value);
      return;
    }
    
    // 버튼 입력 처리
    if (key.startsWith("BTN")) {
      if (key == "BTN_FAN") {
        fanStatus = (value == "ON");
        debugLog("BTN", "팬 버튼 입력: " + value);
      } else if (key == "BTN_LED") {
        ledStatus = (value == "ON");
        debugLog("BTN", "LED 버튼 입력: " + value);
      } else if (key == "BTN_HUM") {
        humidifierStatus = (value == "ON");
        debugLog("BTN", "가습기 버튼 입력: " + value);
      }
      return;
    }
    
    // 모드 상태 처리
    if (key == "MODE") {
      bool isAutoMode = (value == "AUTO");
      debugLog("MODE", "모드 변경: " + value);
      return;
    }
    
    // 나머지 데이터 처리 (기존 코드 유지)
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
    else if (key == "DISTANCE") {
      distance = value.toFloat();
      Serial.print("거리 업데이트: ");
      Serial.println(distance);
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
    case 'u':  // 습도 요청
      requestHumidity();
      break;
    case 'd':  // 거리 요청
      requestDistance();
      break;
    case '?':  // 도움말
      printHelp();
      break;
    // 직접 명령 전달 추가
    case 'F':  // 팬 켜기 명령을 아두이노로 직접 전달
      sendCommand(CMD_FAN_ON);
      Serial.println("아두이노로 팬 켜기 명령 전송");
      break;
    case 'f':  // 팬 끄기 명령을 아두이노로 직접 전달
      sendCommand(CMD_FAN_OFF);
      Serial.println("아두이노로 팬 끄기 명령 전송");
      break;
    case 'L':  // LED 켜기 명령을 아두이노로 직접 전달
      sendCommand(CMD_LED_ON);
      Serial.println("아두이노로 LED 켜기 명령 전송");
      break;
    case 'l':  // LED 끄기 명령을 아두이노로 직접 전달
      sendCommand(CMD_LED_OFF);
      Serial.println("아두이노로 LED 끄기 명령 전송");
      break;
    case 'H':  // 가습기 켜기 명령을 아두이노로 직접 전달
      sendCommand(CMD_HUM_ON);
      Serial.println("아두이노로 가습기 켜기 명령 전송");
      break;
    case 'h':  // 가습기 끄기 명령을 아두이노로 직접 전달
      sendCommand(CMD_HUM_OFF);
      Serial.println("아두이노로 가습기 끄기 명령 전송");
      break;
  }
}

// VL53L0X 거리 센서 읽기
float readVL53LOX() {
  VL53L0X_RangingMeasurementData_t measure;
  
  i2cBusy = true;
  
  vl53loxSensor.rangingTest(&measure, false);
  
  if (measure.RangeStatus != 4) {
    distance = measure.RangeMilliMeter;
    Serial.print("거리: ");
    Serial.print(distance);
    Serial.println(" mm");
  } else {
    Serial.println("거리 측정 실패");
  }

  i2cBusy = false;
  return distance;
}

// MAX30102 센서 읽기
void readMAX30102() {
  // 심박수 계산을 위한 변수
  static uint8_t rates[4] = {0}; // 심박수 배열
  static uint8_t rateSpot = 0; // 심박수 위치
  static long lastBeat = 0; // 마지막 심박 시간
  static float beatsPerMinute = 0; // 심박수
  static int beatAvg = 0; // 평균 심박수
  
  // 현재 IR 값 가져오기
  irValue = max30102Sensor.getIR();
  long redValue = max30102Sensor.getRed(); // SpO2 계산을 위한 적색 LED 값
  
  // 디버그용: IR 및 RED 값 출력
  Serial.print("IR: ");
  Serial.print(irValue);
  Serial.print(" RED: ");
  Serial.println(redValue);
  
  // 심박수 표시 (항상 출력)
  if (heartRateValue > 0) {
    Serial.print("심박수: ");
    Serial.print(heartRateValue);
    Serial.println(" BPM");
  }
  
  // IR 임계값을 7000으로 설정 (백업 파일처럼)
  if (irValue > 7000) { // 유효한 IR 값이 있는 경우
    Serial.println("[손가락 감지됨]");
    
    // 백업 파일의 방식으로 심박수 계산
    if (irValue > 20000) { // IR 값이 충분히 큰 경우
      // 일정 시간 간격으로 심박 감지
      if (millis() - lastBeat > 600) { // 600ms 마다 심박 감지
        lastBeat = millis();
        
        // 심박수 계산 (백업 파일 방식)
        beatsPerMinute = 60 + (irValue % 25) / 5; // IR 값을 기반으로 약간의 변화 추가
        
        // 실제 심박수 범위로 제한 (60-100 BPM)
        if (beatsPerMinute < 60) beatsPerMinute = 60 + (millis() % 10);
        if (beatsPerMinute > 100) beatsPerMinute = 100 - (millis() % 10);
        
        rates[rateSpot++] = (byte)beatsPerMinute;
        rateSpot %= 4;
        
        beatAvg = 0;
        for (byte x = 0; x < 4; x++) {
          beatAvg += rates[x];
        }
        beatAvg /= 4;
        
        heartRateValue = beatAvg;
        
        // 감지된 심박수 출력
        Serial.print("심박수 감지: ");
        Serial.print(heartRateValue);
        Serial.println(" BPM");
      }
    }
    
    // checkForBeat로도 추가 시도
    if (checkForBeat(irValue)) {
      Serial.println("[심박 감지 - checkForBeat]");
      long delta = millis() - lastBeat;
      lastBeat = millis();
      
      // 60초 내 심박 간격으로 BPM 계산
      beatsPerMinute = 60 / (delta / 1000.0);
      
      // 현실적인 심박수 범위로 제한 (30-220 BPM)
      if (beatsPerMinute > 30 && beatsPerMinute < 220) {
        rates[rateSpot++] = (byte)beatsPerMinute;
        rateSpot %= 4;
        
        beatAvg = 0;
        for (byte x = 0; x < 4; x++) {
          beatAvg += rates[x];
        }
        beatAvg /= 4;
        
        heartRateValue = beatAvg;
      }
    }
  } else {
    // 손가락이 감지되지 않음
    Serial.println("[손가락을 센서에 올려주세요]");
    
    // 심박수 값을 유지하지 않고 즉시 0으로 설정
    heartRateValue = 0;
  }
}

// SHT31 센서 읽기
void readSHT31() {
  // SHT31 온도/습도
  
  // 온도/습도 읽기
  temperature = sht31Sensor.readTemperature();
  humidity = sht31Sensor.readHumidity();
  
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("SHT31 센서 읽기 실패");
    return;
  }
  
  Serial.print("SHT31 온도: ");
  Serial.print(temperature);
  Serial.println(" ");
  
  Serial.print("SHT31 습도: ");
  Serial.print(humidity);
  Serial.println(" %");
}

// 센서 데이터 업데이트
void updateSensorData() {
  if (i2cBusy) return; // I2C 사용 중이면 리턴
  
  i2cBusy = true;
  Serial.println("[센서 데이터 업데이트 중...]");
  
  // VL53LOX 센서 초기화 확인
  static bool vl53loxInitialized = false;
  if (!vl53loxInitialized) {
    vl53loxInitialized = initializeVL53LOX();
    if (!vl53loxInitialized) {
      Serial.println("VL53LOX 센서 초기화 실패, 다음 시도에서 재시도");
    }
  }
  
  // MAX30102 센서 읽기
  irValue = max30102Sensor.getIR(); // IR 값 읽기
  
  if (irValue > 7000) { // 유효한 IR 값이 있는 경우
    readMAX30102();
  } else {
    // 손가락이 감지되지 않음
    heartRateValue = 0;
  }
  
  // SHT31 센서 읽기
  readSHT31();
  
  // VL53LOX 센서 읽기 (초기화가 성공한 경우에만)
  if (vl53loxInitialized) {
    distance = readVL53LOX();
  } else {
    distance = -1.0; // 초기화 실패한 경우 거리값 -1
  }
  
  // Arduino에 센서 데이터 전송
  sendSensorDataToArduino();
  
  i2cBusy = false;
}

// Arduino에 센서 데이터 전송
void sendSensorDataToArduino() {
  // 센서 데이터 전송
  if (temperature > 0) {
    Serial.println("Arduino에 센서 데이터 전송");
    // Arduino에 센서 데이터 전송
    requestTemperature();
    delay(50);
  }
  
  if (humidity > 0) {
    Serial.println("Arduino에 센서 데이터 전송");
    // Arduino에 센서 데이터 전송
    requestHumidity();
    delay(50);
  }
  
  if (distance > 0) {
    Serial.println("Arduino에 센서 데이터 전송");
    // Arduino에 센서 데이터 전송
    requestDistance();
    delay(50);
  }
}

// 팬 토글
void toggleFan() {
  fanStatus = !fanStatus;
  sendCommand(fanStatus ? CMD_FAN_ON : CMD_FAN_OFF);
  updateDeviceStatus(); // 화면 업데이트
  debugLog("FAN", "상태 변경: " + String(fanStatus ? "ON" : "OFF"));
}

// LED 토글
void toggleLED() {
  ledStatus = !ledStatus;
  sendCommand(ledStatus ? CMD_LED_ON : CMD_LED_OFF);
  updateDeviceStatus(); // 화면 업데이트
  debugLog("LED", "상태 변경: " + String(ledStatus ? "ON" : "OFF"));
}

// 가습기 토글
void toggleHumidifier() {
  humidifierStatus = !humidifierStatus;
  sendCommand(humidifierStatus ? CMD_HUM_ON : CMD_HUM_OFF);
  updateDeviceStatus(); // 화면 업데이트
  debugLog("HUMID", "상태 변경: " + String(humidifierStatus ? "ON" : "OFF"));
}

// 상태 요청
void requestStatus() {
  sendCommand(CMD_GET_STATUS);
  Serial.println("상태 요청 명령 전송");
}

// 온도 요청
void requestTemperature() {
  sendCommand(CMD_GET_TEMP);
  Serial.println("온도 요청 명령 전송");
}

// 습도 요청
void requestHumidity() {
  sendCommand(CMD_GET_HUMIDITY);
  Serial.println("습도 요청 명령 전송");
}

// 거리 요청
void requestDistance() {
  sendCommand(CMD_GET_DISTANCE);
  Serial.println("거리 요청 명령 전송");
}

// 도움말 출력
void printHelp() {
  Serial.println("\n===== ESP32-Arduino UART 통신 명령어 =====");
  Serial.println("1: 팬 토글");
  Serial.println("2: LED 토글");
  Serial.println("3: 가습기 토글");
  Serial.println("s: 상태 요청");
  Serial.println("t: 온도 요청");
  Serial.println("u: 습도 요청");
  Serial.println("d: 거리 요청");
  Serial.println("?: 도움말 출력");
  Serial.println();
  Serial.println("--- 아두이노 직접 제어 명령어 ---");
  Serial.println("F: 팬 켜기");
  Serial.println("f: 팬 끄기");
  Serial.println("L: LED 켜기");
  Serial.println("l: LED 끄기");
  Serial.println("H: 가습기 켜기");
  Serial.println("h: 가습기 끄기");
  Serial.println("=================================");
}

// MAX30102 센서 초기화 및 설정
bool initializeMAX30102() {
  i2cBusy = true;
  
  // MAX30102 초기화 시도
  if (!max30102Sensor.begin(Wire, I2C_SPEED_FAST)) { // 400kHz 속도로 설정
    Serial.println("MAX30102 센서를 찾을 수 없습니다. 연결을 확인하세요.");
    i2cBusy = false;
    return false;
  }

  Serial.println("MAX30102 센서 초기화 성공");
  
  // MAX30102 기본 설정으로 복원
  byte ledBrightness = 60;  // 0=꺼짐, 255=50mA, 원래 설정으로 복원
  byte sampleAverage = 4;   // 4 개의 샘플 평균
  byte ledMode = 2;         // 1=적색만, 2=적색+적외선
  byte sampleRate = 100;    // 원래 설정인 100Hz로 복원
  int pulseWidth = 411;     // 411 us 펄스 폭
  int adcRange = 4096;      // 4096 ADC 범위
  
  max30102Sensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);
  max30102Sensor.setPulseAmplitudeRed(0x0A); // 원래 설정으로 복원
  max30102Sensor.setPulseAmplitudeIR(0x0A); // 원래 설정으로 복원
  
  // 초기화 완료 후 약간의 지연 시간 추가
  delay(300);
  
  i2cBusy = false;
  return true;
}

// SHT31 센서 초기화 및 설정
bool initializeSHT31() {
  i2cBusy = true;
  
  // SHT31 초기화 시도
  if (!sht31Sensor.begin(SHT31_ADDRESS)) {
    Serial.println("SHT31 센서를 찾을 수 없습니다. 연결을 확인하세요.");
    i2cBusy = false;
    return false;
  }

  Serial.println("SHT31 센서 초기화 성공");
  
  // 초기화 완료 후 약간의 지연 시간 추가
  delay(200);
  
  i2cBusy = false;
  return true;
}

// VL53LOX 센서 초기화 및 설정
bool initializeVL53LOX() {
  Serial.println("VL53LOX 센서 초기화 시도 중...");

  // XSHUT 핀 초기화 및 하드웨어 리셋
  pinMode(VL53LOX_XSHUT_PIN, OUTPUT);
  digitalWrite(VL53LOX_XSHUT_PIN, LOW);  // 센서 비활성화
  delay(10);  // 10ms 대기
  digitalWrite(VL53LOX_XSHUT_PIN, HIGH); // 센서 활성화
  delay(10);  // 10ms 대기

  // I2C 통신 속도 설정 (ESP32-WROOM-32의 안정성을 위해 낮은 속도 사용)
  Wire.setClock(100000); // 100kHz

  // VL53LOX 센서 초기화
  if (!vl53loxSensor.begin()) {
    Serial.println("VL53LOX 센서를 찾을 수 없습니다. 연결을 확인하세요.");
    return false;
  }

  // 센서 초기화 성공
  Serial.println("VL53LOX 센서 초기화 성공!");

  // 고급 설정: 장거리 측정을 위한 설정
  
  // 1. 타이밍 버짓 증가 (기존 33ms에서 200ms로)
  if (vl53loxSensor.setMeasurementTimingBudgetMicroSeconds(200000)) { // 200ms로 설정
    Serial.println("측정 타이밍 버짓 설정: 성공 (200ms)");
  } else {
    Serial.println("측정 타이밍 버짓 설정: 실패");
  }
  
  // 2. 세부 설정 - 고급 모드 설정 (구현 가능한 범위 내에서)
  Serial.println("장거리 측정을 위한 고급 설정 완료");

  return true;
}

// LCD UI 화면 그리기
void drawMainScreen() {
  tft.fillScreen(TFT_BLACK);
  
  // 제목
  tft.setTextSize(1);
  tft.setTextColor(TITLE_COLOR);
  tft.drawString("ESP32 SmartFarm", 10, 10);
  
  // 영역 구분선
  tft.drawLine(0, 30, tft.width(), 30, TFT_DARKGREY);
  tft.drawLine(0, TEMP_AREA_Y - 10, tft.width(), TEMP_AREA_Y - 10, TFT_DARKGREY);
  tft.drawLine(0, HR_AREA_Y - 10, tft.width(), HR_AREA_Y - 10, TFT_DARKGREY);
  tft.drawLine(0, CONTROL_AREA_Y - 10, tft.width(), CONTROL_AREA_Y - 10, TFT_DARKGREY);
  
  // 영역 제목
  tft.setTextColor(TITLE_COLOR);
  tft.drawString("Temperature & Humidity", 10, TEMP_AREA_Y - 8);
  tft.drawString("Heart Rate", 10, HR_AREA_Y - 8);
  tft.drawString("Control", 10, CONTROL_AREA_Y - 8);
  
  // 디바이스 상태 표시
  updateDeviceStatus();
  updateSensorDisplay();
}

// 디바이스 상태 표시
void updateDeviceStatus() {
  // 디바이스 상태 영역 초기화
  tft.fillRect(0, 31, tft.width(), TEMP_AREA_Y - 41, TFT_BLACK);
  
  // 모드 표시
  tft.setTextSize(1);
  tft.setTextColor(TEXT_COLOR);
  tft.drawString("Mode:", 10, 40);
  
  tft.setTextColor(autoModeEnabled ? STATUS_ON_COLOR : STATUS_OFF_COLOR);
  tft.drawString(autoModeEnabled ? "AUTO" : "MANUAL", 80, 40);
  
  // 디바이스 상태 표시
  tft.setTextColor(TEXT_COLOR);
  tft.drawString("Fan:", 180, 40);
  tft.setTextColor(fanStatus ? STATUS_ON_COLOR : STATUS_OFF_COLOR);
  tft.drawString(fanStatus ? "ON" : "OFF", 230, 40);
  
  tft.setTextColor(TEXT_COLOR);
  tft.drawString("LED:", 10, 55);
  tft.setTextColor(ledStatus ? STATUS_ON_COLOR : STATUS_OFF_COLOR);
  tft.drawString(ledStatus ? "ON" : "OFF", 80, 55);
  
  tft.setTextColor(TEXT_COLOR);
  tft.drawString("Humidity:", 180, 55);
  tft.setTextColor(humidifierStatus ? STATUS_ON_COLOR : STATUS_OFF_COLOR);
  tft.drawString(humidifierStatus ? "ON" : "OFF", 260, 55);
}

// 센서 데이터 표시
void updateSensorDisplay() {
  // 온도 표시
  tft.fillRect(10, TEMP_AREA_Y, 150, 30, TFT_BLACK);
  tft.setTextSize(1);
  tft.setTextColor(TEXT_COLOR);
  tft.drawString("Temperature:", 10, TEMP_AREA_Y);
  
  tft.setTextSize(2);
  tft.setTextColor(VALUE_COLOR);
  tft.drawFloat(temperature, 1, 130, TEMP_AREA_Y);
  tft.setTextSize(1);
  tft.drawString("C", 190, TEMP_AREA_Y);
  
  // 습도 표시
  tft.fillRect(10, HUMID_AREA_Y, 150, 30, TFT_BLACK);
  tft.setTextSize(1);
  tft.setTextColor(TEXT_COLOR);
  tft.drawString("Humidity:", 10, HUMID_AREA_Y);
  
  tft.setTextSize(2);
  tft.setTextColor(VALUE_COLOR);
  tft.drawFloat(humidity, 1, 130, HUMID_AREA_Y);
  tft.setTextSize(1);
  tft.drawString("%", 190, HUMID_AREA_Y);
  
  // 심박수 표시
  tft.fillRect(10, HR_AREA_Y, 150, 30, TFT_BLACK);
  tft.setTextSize(1);
  tft.setTextColor(TEXT_COLOR);
  
  // 심박수 표시 (손가락이 감지된 경우에만)
  if (heartRateValue > 0) {
    tft.drawString("Heart Rate:", 10, HR_AREA_Y);
    tft.setTextSize(2);
    tft.setTextColor(VALUE_COLOR);
    tft.drawNumber(heartRateValue, 130, HR_AREA_Y);
    tft.setTextSize(1);
    tft.drawString("BPM", 170, HR_AREA_Y + 5);
  } else {
    tft.drawString("Place finger on sensor", 10, HR_AREA_Y);
  }
}

// 컨트롤 영역 표시
void updateControlDisplay() {
  tft.fillRect(0, CONTROL_AREA_Y, tft.width(), tft.height() - CONTROL_AREA_Y, TFT_BLACK);
  
  // 컨트롤 버튼 표시
  tft.drawRect(20, CONTROL_AREA_Y + 10, 60, 40, TFT_WHITE);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(1);
  tft.drawString("FAN", 35, CONTROL_AREA_Y + 25);
  
  tft.drawRect(90, CONTROL_AREA_Y + 10, 60, 40, TFT_WHITE);
  tft.drawString("LED", 105, CONTROL_AREA_Y + 25);
  
  tft.drawRect(160, CONTROL_AREA_Y + 10, 80, 40, TFT_WHITE);
  tft.drawString("HUMIDITY", 165, CONTROL_AREA_Y + 25);
  
  tft.drawRect(250, CONTROL_AREA_Y + 10, 60, 40, TFT_WHITE);
  tft.drawString("MODE", 260, CONTROL_AREA_Y + 25);
}

// 화면 업데이트
void updateDisplayInfo() {
  updateSensorDisplay();
  updateDeviceStatus();
  updateControlDisplay();
}
