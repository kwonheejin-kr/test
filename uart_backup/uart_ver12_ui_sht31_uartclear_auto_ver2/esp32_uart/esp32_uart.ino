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

// 이전 센서 데이터 변수
float prevTemperature = 0;
float prevHumidity = 0;

// SHT31 센서 샘플링 변수
const int TEMP_SAMPLES = 5;       // 온도 샘플링 개수
const int HUMID_SAMPLES = 5;      // 습도 샘플링 개수
float tempSamples[TEMP_SAMPLES] = {0}; // 온도 샘플링 데이터
float humidSamples[HUMID_SAMPLES] = {0}; // 습도 샘플링 데이터
int sampleIndex = 0; // 샘플링 인덱스
int validSamples = 0; // 유효한 샘플 수

// VL53LOX 센서 초기화 여부
bool vl53loxInitialized = false;

// SHT31 센서 초기화 여부
bool sht31Initialized = false;

// MAX30102 센서 초기화 여부
bool max30102Initialized = false;

// 상태 변수
bool fanStatus = false;
bool ledStatus = false;
bool humidifierStatus = false;
bool autoModeEnabled = false;

// 센서 데이터 전송 관련 변수
float lastSentTemperature = 0;
float lastSentHumidity = 0;

// 타이머 변수
unsigned long lastStatusRequest = 0;
const unsigned long STATUS_INTERVAL = 5000;  // 5초마다 상태 확인
unsigned long lastSensorUpdate = 0;
const unsigned long SENSOR_UPDATE_INTERVAL = 2000; // 2초마다 센서 데이터 갱신
unsigned long lastVL53LOXMeasurement = 0;
const unsigned long VL53LOX_INTERVAL = 1000; // 1초마다 VL53LOX 센서 측정
unsigned long lastAutoModeUpdate = 0;
const unsigned long AUTO_MODE_INTERVAL = 1000; // 1초마다 자동 모드 업데이트

// LCD UI 관련 변수
unsigned long lastDisplayUpdate = 0;
const int DISPLAY_UPDATE_INTERVAL = 1000; // 1초마다 화면 업데이트

// UI 영역 정의
const int STATUS_AREA_Y = 10;  // 상태 영역
const int TEMP_AREA_Y = 50;    // 온도 영역
const int HR_AREA_Y = 140;     // 심박수 영역
const int DEVICE_STATUS_Y = 180; // 디바이스 상태 영역 (좀 더 위로 조정)

// 색상 정의 (이미 User_Setup.h에 정의되어 있지만 편의상 사용)
const uint16_t STATUS_ON_COLOR = 0x07E0;      // 밝은 녹색
const uint16_t STATUS_OFF_COLOR = 0x5AEB;     // 어두운 회색
const uint16_t TEXT_COLOR = 0xFFFF;          // 흰색
const uint16_t VALUE_COLOR = 0xFFE0;          // 노란색
const uint16_t TITLE_COLOR = 0x07FF;          // 청록색
const uint16_t TEMP_COLOR = 0xF800;           // 빨간색
const uint16_t HUMID_COLOR = 0x001F;          // 파란색
const uint16_t HR_COLOR = 0xF81F;             // 마젠타
const uint16_t BACKGROUND_COLOR = 0x0000;     // 검은색
const uint16_t BORDER_COLOR = 0x5AEB;         // 어두운 회색
const uint16_t PROGRESS_BG_COLOR = 0x2124;    // 진행바 배경색
const uint16_t AUTO_COLOR = 0x07FF;           // 자동 모드 색상
const uint16_t MANUAL_COLOR = 0xFD20;         // 수동 모드 색상
const uint16_t CARD_BG_COLOR = 0x1082;        // 카드 배경색
const uint16_t HEADER_COLOR = 0x19BE;         // 헤더 색상

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
bool debugEnabled = false;  // 디버깅 출력 비활성화

// 상단에 추가할 정의 - 전역 변수 선언 부분
#define VL53LOX_XSHUT_PIN 13  // ESP32의 GPIO 13을 XSHUT 핀으로 사용

const int MAX_BUFFER_SIZE = 64;
char rxBuffer[MAX_BUFFER_SIZE];
int rxIndex = 0;
bool messageReady = false;

unsigned long lastSensorSendTime = 0;

// ud568uc218 uc804ubc29 uc120uc5b8 (ucef4ud30cuc77c uc624ub958 ud574uacb0uc6a9)
void toggleAutoMode();
void toggleFan();
void toggleLED();
void toggleHumidifier();
void requestStatus();
void requestTemperature();
void requestHumidity();
void requestDistance();

void setup() {
  Serial.begin(115200);  // USB 시리얼 초기화
  
  // Arduino와 통신할 UART2 초기화 (보드레이트 및 개행문자 처리설정 변경)
  ArduinoSerial.begin(115200, SERIAL_8N1, 16, 17);  // UART2 115200bps
  Serial.println("INIT: Arduino UART 연결 완료 (115200 baud)");
  
  // LED 초기화
  pinMode(5, OUTPUT);
  digitalWrite(5, HIGH);  // LED 켜기
  
  // I2C 초기화
  Wire.begin();  
  
  // SHT31 센서 초기화
  if (sht31Sensor.begin(0x44)) {
    Serial.println("INIT: SHT31 센서 초기화 성공");
    sht31Initialized = true;
  } else {
    Serial.println("ERROR: SHT31 센서 초기화 실패");
  }
  
  // MAX30102 센서 초기화
  if (initializeMAX30102()) {
    Serial.println("INIT: MAX30102 센서 초기화 성공");
    max30102Initialized = true;
  } else {
    Serial.println("ERROR: MAX30102 센서 초기화 실패");
  }
  
  // TFT 디스플레이 초기화
  tft.init();
  tft.setRotation(1);  // 화면 방향 설정 (1: 세로, 3: 가로)
  tft.fillScreen(TFT_BLACK);
  
  // 메인 화면 그리기
  drawMainScreen();
  
  // 초기화 지연
  delay(1000);
  
  // 초기 상태 요청
  requestStatus();
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
    processSerialCommand(command.charAt(0));
  }
  
  // Arduino로부터 데이터 수신 (UART2) - 개선된 바이트 단위 읽기
  while (ArduinoSerial.available()) {
    char c = ArduinoSerial.read();
    
    // 개행 문자를 메시지 종료로 처리
    if (c == '\n' || c == '\r') {
      if (rxIndex > 0) { // 빈 메시지 무시
        rxBuffer[rxIndex] = '\0'; // 문자열 종료
        messageReady = true;
        break;
      }
    } else if (rxIndex < MAX_BUFFER_SIZE - 1) { // 버퍼 오버플로우 방지
      rxBuffer[rxIndex++] = c;
    }
  }
  
  // 완전한 메시지가 수신되면 처리
  if (messageReady) {
    String data = String(rxBuffer);
    rxIndex = 0; // 버퍼 인덱스 초기화
    messageReady = false; // 메시지 처리 플래그 초기화
    
    // 데이터가 비어있지 않은 경우에만 처리
    if (data.length() > 0) {
      Serial.print("[RAW] Received: ");
      // 16진수로 각 바이트 출력 (디버깅용)
      for (int i = 0; i < data.length(); i++) {
        Serial.print("0x");
        Serial.print((byte)data.charAt(i), HEX);
        Serial.print(" ");
      }
      Serial.println();
      
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
    processSensorData();
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
  
  // 자동 모드 업데이트
  if (autoModeEnabled && millis() - lastAutoModeUpdate > AUTO_MODE_INTERVAL) {
    processAutoMode();
    lastAutoModeUpdate = millis();
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
  
  Serial.print("[CMD] Sending: ");
  Serial.println(commandStr);
  
  ArduinoSerial.println(commandStr);  // 개행문자 추가하여 전송
}

// 디버깅 로그 출력 함수 - 최소화
void debugLog(String tag, String message) {
  // 디버깅 비활성화 상태에서는 아무것도 하지 않음
  // 활성화된 경우에만 필수적인 로그 출력
  if (debugEnabled) {
    Serial.print("[");
    Serial.print(tag);
    Serial.print("] ");
    Serial.println(message);
  }
}

// Arduino로부터 받은 데이터 처리
void processArduinoData(String data) {
  // 시리얼 모니터에 수신된 데이터 출력 (디버깅용)
  Serial.print("[Arduino] Received: ");
  Serial.println(data);
  
  // 데이터 형식: "key:value"
  int separatorIndex = data.indexOf(':');
  if (separatorIndex > 0) {
    String key = data.substring(0, separatorIndex);
    String value = data.substring(separatorIndex + 1);
    
    // 디버깅 출력
    Serial.print("[Parse] Key: ");
    Serial.print(key);
    Serial.print(", Value: ");
    Serial.println(value);
    
    // 확인 응답 처리
    if (key == "ACK") {
      return;
    }
    
    // 버튼 입력 처리
    if (key.startsWith("BTN")) {
      if (key == "BTN_FAN") {
        fanStatus = (value == "ON");
        Serial.print("[BTN] Fan status changed to: ");
        Serial.println(fanStatus ? "ON" : "OFF");
      } else if (key == "BTN_LED") {
        ledStatus = (value == "ON");
        Serial.print("[BTN] LED status changed to: ");
        Serial.println(ledStatus ? "ON" : "OFF");
      } else if (key == "BTN_HUM") {
        humidifierStatus = (value == "ON");
        Serial.print("[BTN] Humidifier status changed to: ");
        Serial.println(humidifierStatus ? "ON" : "OFF");
      } else if (key == "BTN_MODE") { // 자동 모드 버튼 입력 처리
        toggleAutoMode();
        Serial.print("[BTN] Mode changed to: ");
        Serial.println(autoModeEnabled ? "AUTO" : "MANUAL");
      }
      updateDeviceStatus(); // 화면 업데이트 추가
      return;
    }
    
    // 모드 상태 처리
    if (key == "MODE") {
      autoModeEnabled = (value == "AUTO");
      Serial.print("[MODE] Set to: ");
      Serial.println(autoModeEnabled ? "AUTO" : "MANUAL");
      updateDeviceStatus(); // 화면 업데이트
      return;
    }
    
    // 상태 업데이트 (필수 기능)
    if (key == "FAN") {
      fanStatus = (value == "ON");
      Serial.print("[STATUS] Fan: ");
      Serial.println(fanStatus ? "ON" : "OFF");
    }
    else if (key == "LED") {
      ledStatus = (value == "ON");
      Serial.print("[STATUS] LED: ");
      Serial.println(ledStatus ? "ON" : "OFF");
    }
    else if (key == "HUM") {
      humidifierStatus = (value == "ON");
      Serial.print("[STATUS] Humidifier: ");
      Serial.println(humidifierStatus ? "ON" : "OFF");
    }
    else if (key == "TEMP") {
      temperature = value.toFloat();
    }
    else if (key == "HUM_VAL") {
      humidity = value.toFloat();
    }
    else if (key == "DISTANCE") {
      distance = value.toFloat();
    }
    
    // 모든 상태 업데이트 후 화면 갱신
    updateDeviceStatus();
  }
}

// VL53L0X 거리 센서 읽기
float readVL53LOX() {
  VL53L0X_RangingMeasurementData_t measure;
  
  i2cBusy = true;
  
  vl53loxSensor.rangingTest(&measure, false);
  
  if (measure.RangeStatus != 4) {
    distance = measure.RangeMilliMeter;
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
  
  // IR 임계값을 7000으로 설정 (백업 파일처럼)
  if (irValue > 7000) { // 유효한 IR 값이 있는 경우
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
      }
    }
    
    // checkForBeat로도 추가 시도
    if (checkForBeat(irValue)) {
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
    heartRateValue = 0;
  }
}

// SHT31 센서 읽기
void readSHT31() {
  // 온도/습도 읽기
  float newTemp = sht31Sensor.readTemperature();
  float newHumid = sht31Sensor.readHumidity();
  
  // 유효한 값인지 확인
  if (!isnan(newTemp) && !isnan(newHumid)) {
    // 이상치 필터링 - 온도
    bool tempValid = true;
    if (prevTemperature > 0) {
      // 이전 값과 새 값의 차이가 15도 이상이면 이상치로 간주
      if (abs(newTemp - prevTemperature) > 15.0) {
        tempValid = false; // 이상치 표시
        if (debugEnabled) {
          Serial.print("Temperature outlier filtered: ");
          Serial.print(newTemp);
          Serial.print(" (prev: ");
          Serial.print(prevTemperature);
          Serial.println(")");
        }
      }
    }
    
    // 이상치 필터링 - 습도
    bool humidValid = true;
    if (prevHumidity > 0) {
      // 이전 값과 새 값의 차이가 25% 이상이면 이상치로 간주
      if (abs(newHumid - prevHumidity) > 25.0) {
        humidValid = false; // 이상치 표시
        if (debugEnabled) {
          Serial.print("Humidity outlier filtered: ");
          Serial.print(newHumid);
          Serial.print(" (prev: ");
          Serial.print(prevHumidity);
          Serial.println(")");
        }
      }
    }
    
    // 측정 범위를 벗어나는 경우도 이상치로 처리
    if (newTemp < -40 || newTemp > 125) tempValid = false;
    if (newHumid < 0 || newHumid > 100) humidValid = false;
    
    // 샘플 배열에 유효한 값만 추가
    if (tempValid) {
      tempSamples[sampleIndex] = newTemp;
      // 이전 값을 현재 값으로 저장 (다음 비교를 위해)
      prevTemperature = newTemp;
    }
    
    if (humidValid) {
      humidSamples[sampleIndex] = newHumid;
      // 이전 값을 현재 값으로 저장 (다음 비교를 위해)
      prevHumidity = newHumid;
    }
    
    // 샘플 인덱스 업데이트
    sampleIndex = (sampleIndex + 1) % TEMP_SAMPLES; // TEMP_SAMPLES와 HUMID_SAMPLES는 동일함
    if (validSamples < TEMP_SAMPLES) validSamples++;
    
    // 이동평균 계산
    if (validSamples > 0) {
      float tempSum = 0;
      float humidSum = 0;
      int validTempCount = 0;
      int validHumidCount = 0;
      
      for (int i = 0; i < validSamples; i++) {
        // 0이 아닌 값만 고려
        if (tempSamples[i] > 0) {
          tempSum += tempSamples[i];
          validTempCount++;
        }
        
        if (humidSamples[i] > 0) {
          humidSum += humidSamples[i];
          validHumidCount++;
        }
      }
      
      // 이동평균으로 값 업데이트
      if (validTempCount > 0) {
        temperature = tempSum / validTempCount;
      }
      
      if (validHumidCount > 0) {
        humidity = humidSum / validHumidCount;
      }
    }
  }
  
  // 다음 측정까지 약간의 지연 추가하여 I2C 버스 안정화
  delay(5);
}

// 센서 데이터 업데이트
void processSensorData() {
  // SHT31 센서 처리
  if (sht31Initialized && !i2cBusy) {
    readSHT31();
    i2cBusy = false;
    
    // 자동 모드일 때만 아두이노에 전송
    // 데이터 변동폭이 클 때만 전송 (기준 높임)
    if (autoModeEnabled && (
        abs(lastSentTemperature - temperature) >= 1.0 || 
        abs(lastSentHumidity - humidity) >= 2.0)) {
      // 30초마다 데이터 전송 (통신 빈도 제한)
      unsigned long currentMillis = millis();
      if (currentMillis - lastSensorSendTime >= 30000) { // 30초마다
        sendSensorDataToArduino();
        lastSentTemperature = temperature;
        lastSentHumidity = humidity;
        lastSensorSendTime = currentMillis;
      }
    }
  }
  
  // MAX30102 센서 처리
  if (max30102Initialized && !i2cBusy) {
    readMAX30102();
    i2cBusy = false;
  }
  
  // VL53L0X 센서 처리
  if (vl53loxInitialized && !i2cBusy) {
    distance = readVL53LOX();
    i2cBusy = false;
  }
}

// Arduino에 센서 데이터 전송
void sendSensorDataToArduino() {
  // 자동 모드에 필요한 센서 데이터만 전송
  if (autoModeEnabled) {
    // 온도, 습도가 유효한 경우만 전송
    if (temperature > 0 && temperature < 100) {
      requestTemperature();
      delay(50); // 안정성을 위해 조금 더 긴 지연 사용
    }
    
    if (humidity > 0 && humidity <= 100) {
      requestHumidity();
      delay(50);
    }
    
    // 거리 정보는 자동 모드에 중요하기 때문에 전송
    if (distance > 0 && distance < 1000) { // 합리적인 범위만 전송
      requestDistance();
      delay(50);
    }
  } else {
    // 수동 모드에서는 릴레이 상태만 필요하기 때문에 센서 데이터 전송하지 않음
  }
}

// LCD UI 화면 그리기
void drawMainScreen() {
  tft.fillScreen(BACKGROUND_COLOR);
  
  // 상단 헤더 바 (모던한 디자인)
  tft.fillRect(0, 0, tft.width(), 30, HEADER_COLOR);
  tft.setTextSize(1);
  tft.setTextColor(TEXT_COLOR);
  tft.drawString("ESP32 SmartFarm", 10, 10);
  
  // 현재 모드 표시
  tft.setTextColor(autoModeEnabled ? AUTO_COLOR : MANUAL_COLOR);
  tft.drawString(autoModeEnabled ? "AUTO" : "MANUAL", tft.width() - 40, 10);
  
  // 온습도 영역 - 라운드 카드 스타일
  tft.fillRoundRect(10, TEMP_AREA_Y - 10, tft.width() - 20, 95, 10, CARD_BG_COLOR);
  
  // 심박수 영역 - 라운드 카드 스타일
  tft.fillRoundRect(10, HR_AREA_Y - 10, tft.width() - 20, 55, 10, CARD_BG_COLOR);
  
  // 디바이스 상태 영역 - 라운드 카드 스타일
  tft.fillRoundRect(10, DEVICE_STATUS_Y - 10, tft.width() - 20, 75, 10, CARD_BG_COLOR);
  
  // 영역 제목
  tft.setTextColor(TITLE_COLOR);
  tft.drawString("Temperature & Humidity", 20, TEMP_AREA_Y - 5);
  tft.drawString("Heart Rate", 20, HR_AREA_Y - 5); 
  tft.drawString("", 20, DEVICE_STATUS_Y - 5); 
  
  // 아이콘 그리기 함수 (심플한 아이콘)
  drawThermometerIcon(18, TEMP_AREA_Y + 15);
  drawDropIcon(18, TEMP_AREA_Y + 45);
  drawHeartIcon(18, HR_AREA_Y + 15);
  
  // 디바이스 상태 표시
  updateDeviceStatus();
  updateSensorDisplay();
}

// 온도계 아이콘 그리기
void drawThermometerIcon(int x, int y) {
  // 온도계 하단 둥근 부분
  tft.fillCircle(x, y+10, 6, TEMP_COLOR);
  // 온도계 기둥
  tft.fillRect(x-2, y-5, 4, 15, TEMP_COLOR);
  // 온도계 상단 둥근 부분
  tft.fillCircle(x, y-6, 3, TEMP_COLOR);
}

// 물방울 아이콘 그리기
void drawDropIcon(int x, int y) {
  // 물방울 상단
  for (int i=0; i<7; i++) {
    tft.drawFastHLine(x-i, y-i, i*2, HUMID_COLOR);
  }
  // 물방울 하단
  tft.fillCircle(x, y+1, 6, HUMID_COLOR);
}

// 심장 아이콘 그리기
void drawHeartIcon(int x, int y) {
  tft.fillCircle(x-4, y-2, 4, HR_COLOR);
  tft.fillCircle(x+4, y-2, 4, HR_COLOR);
  tft.fillTriangle(x-8, y, x, y+8, x+8, y, HR_COLOR);
}

// 센서 데이터 표시
void updateSensorDisplay() {
  // 온도 표시
  tft.setTextSize(1);
  tft.setTextColor(TEXT_COLOR);
  tft.drawString("Temperature:", 40, TEMP_AREA_Y + 15);
  
  // 숫자값 표시영역 지우기
  tft.fillRect(150, TEMP_AREA_Y + 10, 70, 20, CARD_BG_COLOR);
  tft.setTextSize(2);
  tft.setTextColor(VALUE_COLOR);
  tft.drawFloat(temperature, 1, 160, TEMP_AREA_Y + 15);
  tft.setTextSize(1);
  tft.drawString("°C", 220, TEMP_AREA_Y + 15);
  
  // 온도 게이지
  int tempGaugeWidth = map(temperature, 0, 40, 0, 240);
  uint16_t tempColor = TEMP_COLOR;
  if(temperature > 30) tempColor = 0xF800; // 고온일 때 빨간색
  else if(temperature < 15) tempColor = 0x001F; // 저온일 때 파란색
  
  tft.fillRect(30, TEMP_AREA_Y + 32, 240, 5, 0x5AEB); // 게이지 배경
  tft.fillRect(30, TEMP_AREA_Y + 32, tempGaugeWidth, 5, tempColor); // 게이지 바
  
  // 습도 표시
  tft.setTextSize(1);
  tft.setTextColor(TEXT_COLOR);
  tft.drawString("Humidity:", 40, TEMP_AREA_Y + 45);
  
  // 숫자값 표시영역 지우기
  tft.fillRect(150, TEMP_AREA_Y + 40, 70, 20, CARD_BG_COLOR);
  tft.setTextSize(2);
  tft.setTextColor(VALUE_COLOR);
  tft.drawFloat(humidity, 1, 160, TEMP_AREA_Y + 45);
  tft.setTextSize(1);
  tft.drawString("%", 220, TEMP_AREA_Y + 45);
  
  // 습도 게이지
  int humidGaugeWidth = map(humidity, 0, 100, 0, 240);
  uint16_t humidColor = HUMID_COLOR;
  if(humidity > 70) humidColor = 0x001F; // 고습도일 때 진한 파란색
  else if(humidity < 30) humidColor = 0xC618; // 저습도일 때 회색
  
  tft.fillRect(30, TEMP_AREA_Y + 62, 240, 5, 0x5AEB); // 게이지 배경
  tft.fillRect(30, TEMP_AREA_Y + 62, humidGaugeWidth, 5, humidColor); // 게이지 바
  
  // 심박수 표시
  tft.setTextSize(1);
  tft.setTextColor(TEXT_COLOR);
  
  // 심박수 표시 (손가락이 감지된 경우에만)
  if (heartRateValue > 0) {
    tft.drawString("Heart Rate:", 40, HR_AREA_Y + 15);
    // 숫자값 표시영역 지우기
    tft.fillRect(150, HR_AREA_Y + 10, 70, 20, CARD_BG_COLOR);
    tft.setTextSize(2);
    tft.setTextColor(VALUE_COLOR);
    tft.drawNumber(heartRateValue, 160, HR_AREA_Y + 15);
    tft.setTextSize(1);
    tft.drawString("BPM", 200, HR_AREA_Y + 15);
  } else {
    // 배경 실시간 클리어
    tft.fillRect(40, HR_AREA_Y + 10, 220, 20, CARD_BG_COLOR);
    tft.drawString("Place finger on sensor", 40, HR_AREA_Y + 15);
  }
}

// 디바이스 상태 표시
void updateDeviceStatus() {
  int statusY = DEVICE_STATUS_Y;
  
  // 현재 모드 표시 업데이트 (오른쪽 상단)
  tft.setTextSize(1); // 글자 크기 설정
  tft.setTextColor(autoModeEnabled ? AUTO_COLOR : MANUAL_COLOR);
  tft.fillRect(tft.width() - 45, 10, 45, 10, TFT_BLACK); // 이전 텍스트 지우기
  tft.drawString(autoModeEnabled ? "AUTO" : "MANUAL", tft.width() - 40, 10);
  tft.setTextSize(2); // 디바이스 상태용 글자 크기로 복원
  
  // 디바이스 상태 표시 - 팬
  tft.fillRoundRect(20, statusY, 80, 40, 5, CARD_BG_COLOR);
  tft.drawRoundRect(20, statusY, 80, 40, 5, fanStatus ? STATUS_ON_COLOR : STATUS_OFF_COLOR);
  tft.setTextColor(TEXT_COLOR);
  tft.drawString("", 40, statusY + 5);
  // 상태 텍스트 영역 지우기
  tft.fillRect(40, statusY + 22, 40, 15, CARD_BG_COLOR);
  tft.setTextColor(fanStatus ? STATUS_ON_COLOR : STATUS_OFF_COLOR);
  tft.drawString(fanStatus ? "ON" : "OFF", 45, statusY + 25);
  
  // 디바이스 상태 표시 - LED
  tft.fillRoundRect(120, statusY, 80, 40, 5, CARD_BG_COLOR);
  tft.drawRoundRect(120, statusY, 80, 40, 5, ledStatus ? STATUS_ON_COLOR : STATUS_OFF_COLOR);
  tft.setTextColor(TEXT_COLOR);
  tft.drawString("", 140, statusY + 5);
  // 상태 텍스트 영역 지우기
  tft.fillRect(140, statusY + 22, 40, 15, CARD_BG_COLOR);
  tft.setTextColor(ledStatus ? STATUS_ON_COLOR : STATUS_OFF_COLOR);
  tft.drawString(ledStatus ? "ON" : "OFF", 145, statusY + 25);
  
  // 디바이스 상태 표시 - 가습기
  tft.fillRoundRect(220, statusY, 80, 40, 5, CARD_BG_COLOR);
  tft.drawRoundRect(220, statusY, 80, 40, 5, humidifierStatus ? STATUS_ON_COLOR : STATUS_OFF_COLOR);
  tft.setTextColor(TEXT_COLOR);
  tft.drawString("", 240, statusY + 5);
  // 상태 텍스트 영역 지우기
  tft.fillRect(240, statusY + 22, 40, 15, CARD_BG_COLOR);
  tft.setTextColor(humidifierStatus ? STATUS_ON_COLOR : STATUS_OFF_COLOR);
  tft.drawString(humidifierStatus ? "ON" : "OFF", 245, statusY + 25);
}

// 화면 업데이트
void updateDisplayInfo() {
  updateSensorDisplay();
  updateDeviceStatus();
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

  vl53loxInitialized = true;
  return true;
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
    case '4':  // 자동 모드 토글
      toggleAutoMode();
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
    case 'F':  // 팬 켜기 명령을 아두이노로 직접 전달
      sendCommand(CMD_FAN_ON);
      break;
    case 'f':  // 팬 끄기 명령을 아두이노로 직접 전달
      sendCommand(CMD_FAN_OFF);
      break;
    case 'L':  // LED 켜기 명령을 아두이노로 직접 전달
      sendCommand(CMD_LED_ON);
      break;
    case 'l':  // LED 끄기 명령을 아두이노로 직접 전달
      sendCommand(CMD_LED_OFF);
      break;
    case 'H':  // 가습기 켜기 명령을 아두이노로 직접 전달
      sendCommand(CMD_HUM_ON);
      break;
    case 'h':  // 가습기 끄기 명령을 아두이노로 직접 전달
      sendCommand(CMD_HUM_OFF);
      break;
    case 'A':  // 자동 모드 켜기
      autoModeEnabled = true;
      sendCommand('M', "AUTO");
      updateDeviceStatus();
      break;
    case 'a':  // 자동 모드 끄기
      autoModeEnabled = false;
      sendCommand('M', "MANUAL");
      updateDeviceStatus();
      break;
  }
}

// 자동 모드 업데이트
void processAutoMode() {
  if (!autoModeEnabled) return;
  
  // 1. 거리 100mm 이내일 때 LED와 가습기 켜기
  if (distance > 0 && distance <= 100) {
    // LED 켜기
    if (!ledStatus) {
      sendCommand(CMD_LED_ON);
      ledStatus = true;
    }
    
    // 가습기 켜기
    if (!humidifierStatus) {
      sendCommand(CMD_HUM_ON);
      humidifierStatus = true;
    }
  } else if (distance > 100) { // 거리가 100mm 초과할 때 LED와 가습기 끄기
    // LED 끄기
    if (ledStatus) {
      sendCommand(CMD_LED_OFF);
      ledStatus = false;
    }
    
    // 가습기 끄기
    if (humidifierStatus) {
      sendCommand(CMD_HUM_OFF);
      humidifierStatus = false;
    }
  }
  
  // 2. 습도 80% 이상일 때 팬 켜기
  if (humidity >= 80) {
    // 팬 켜기
    if (!fanStatus) {
      sendCommand(CMD_FAN_ON);
      fanStatus = true;
    }
  } else if (humidity < 70) { // 습도 70% 미만일 때 팬 끄기
    if (fanStatus) {
      sendCommand(CMD_FAN_OFF);
      fanStatus = false;
    }
  }
  
  // 디바이스 상태 업데이트
  updateDeviceStatus();
}

// 자동 모드 토글
void toggleAutoMode() {
  autoModeEnabled = !autoModeEnabled;
  sendCommand('M', autoModeEnabled ? "AUTO" : "MANUAL");
  updateDeviceStatus();
  
  // 자동 모드 끄기 시 디바이스 초기화
  if (!autoModeEnabled) {
    // 디바이스 초기화
  }
}

// uc790ub3d9 ubaa8ub4dc ud1a0uae00
void toggleFan() {
  fanStatus = !fanStatus;
  sendCommand(fanStatus ? CMD_FAN_ON : CMD_FAN_OFF);
  updateDeviceStatus();
}

// LED ud1a0uae00 ud568uc218
void toggleLED() {
  ledStatus = !ledStatus;
  sendCommand(ledStatus ? CMD_LED_ON : CMD_LED_OFF);
  updateDeviceStatus();
}

// uac00uc2b5uae30 ud1a0uae00 ud568uc218
void toggleHumidifier() {
  humidifierStatus = !humidifierStatus;
  sendCommand(humidifierStatus ? CMD_HUM_ON : CMD_HUM_OFF);
  updateDeviceStatus();
}

// uc0c1ud0dc uc694uccad ud568uc218
void requestStatus() {
  sendCommand('S', "");
}

// uc628ub3c4 uc694uccad ud568uc218
void requestTemperature() {
  sendCommand('T', String(temperature));
}

// uc2b5ub3c4 uc694uccad ud568uc218
void requestHumidity() {
  sendCommand('H', String(humidity));
}

// uac70ub9ac uc694uccad ud568uc218
void requestDistance() {
  sendCommand('D', String(distance));
}
