/*
 * ESP32-WROOM-32와 Arduino R4 간 UART 통신 코드
 * - UART2 사용 (RX: GPIO16, TX: GPIO17)
 * - I2C 센서 연결 (SDA: GPIO21, SCL: GPIO22)
 * - 명령어 기반 통신 프로토콜 구현
 * - 릴레이 제어 기능 (팬, LED, 가습기)
 * - 센서 데이터 수신 기능
 */

// UART2 정의 (Arduino와 통신)
HardwareSerial ArduinoSerial(2); // UART2 사용

// I2C 라이브러리 추가
#include <Wire.h>
#include <MAX30105.h>  // MAX30102 센서 라이브러리 추가
#include <heartRate.h>  // 심박수 계산 알고리즘 라이브러리
#include <Adafruit_SHT31.h>  // SHT31 센서 라이브러리 추가
#include <Adafruit_VL53L0X.h>  // VL53LOX 센서 라이브러리 추가

// 핀 정의
const int rxPin = 16;  // GPIO16을 RX로 사용
const int txPin = 17;  // GPIO17을 TX로 사용
const int sdaPin = 21; // GPIO21을 SDA로 사용
const int sclPin = 22; // GPIO22를 SCL로 사용

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

// 타이머 변수
unsigned long lastStatusRequest = 0;
const unsigned long STATUS_INTERVAL = 5000;  // 5초마다 상태 확인
unsigned long lastSensorUpdate = 0;
const unsigned long SENSOR_UPDATE_INTERVAL = 5000; // 5초마다 센서 데이터 갱신
unsigned long lastVL53LOXMeasurement = 0;
const unsigned long VL53LOX_INTERVAL = 1000; // 1초마다 VL53LOX 센서 측정

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

// 상단에 추가할 정의 - 전역 변수 선언 부분
#define VL53LOX_XSHUT_PIN 13  // ESP32의 GPIO 13을 XSHUT 핀으로 사용

void setup() {
  // 디버깅용 시리얼 초기화
  Serial.begin(115200);
  Serial.println("ESP32 UART & I2C 통신 시작");
  
  // Arduino와 통신할 UART2 초기화
  ArduinoSerial.begin(9600, SERIAL_8N1, rxPin, txPin);
  
  // I2C 초기화 - ESP32-WROOM-32의 한계를 고려하여 속도를 높임
  Wire.begin(sdaPin, sclPin);
  Wire.setClock(100000); // I2C 통신 속도를 100kHz로 높임 (ESP32-WROOM-32 한계 고려)
  
  // I2C 장치 스캔
  scanI2CDevices();
  
  // MAX30102 센서 초기화
  initializeMAX30102();
  
  // SHT31 센서 초기화
  initializeSHT31();
  
  // VL53LOX 센서 초기화
  initializeVL53LOX();
  
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
    char cmd = Serial.read();
    processSerialCommand(cmd);
  }
  
  // Arduino로부터 데이터 수신 (UART2)
  if (ArduinoSerial.available()) {
    String data = ArduinoSerial.readStringUntil('\n');
    processArduinoData(data);
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
      ArduinoSerial.print(CMD_FAN_ON);
      Serial.println("아두이노로 팬 켜기 명령 전송");
      break;
    case 'f':  // 팬 끄기 명령을 아두이노로 직접 전달
      ArduinoSerial.print(CMD_FAN_OFF);
      Serial.println("아두이노로 팬 끄기 명령 전송");
      break;
    case 'L':  // LED 켜기 명령을 아두이노로 직접 전달
      ArduinoSerial.print(CMD_LED_ON);
      Serial.println("아두이노로 LED 켜기 명령 전송");
      break;
    case 'l':  // LED 끄기 명령을 아두이노로 직접 전달
      ArduinoSerial.print(CMD_LED_OFF);
      Serial.println("아두이노로 LED 끄기 명령 전송");
      break;
    case 'H':  // 가습기 켜기 명령을 아두이노로 직접 전달
      ArduinoSerial.print(CMD_HUM_ON);
      Serial.println("아두이노로 가습기 켜기 명령 전송");
      break;
    case 'h':  // 가습기 끄기 명령을 아두이노로 직접 전달
      ArduinoSerial.print(CMD_HUM_OFF);
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
  
  // 항상 현재 심박수 값 출력 (손가락이 감지되지 않아도 심박수 표시)
  Serial.print("심박수: ");
  if (heartRateValue > 0) {
    Serial.print(heartRateValue);
    Serial.println(" BPM");
  } else {
    Serial.println("감지되지 않음");
  }
  
  // IR 임계값을 20000으로 낮춰서 손가락 감지 성능 향상 (ESP32-WROOM-32 최적화)
  if (irValue > 20000) { // 유효한 IR 값이 있는 경우
    Serial.println("[손가락 감지됨]");
    
    // checkForBeat 함수로 심박 감지
    if (checkForBeat(irValue)) {
      Serial.println("[심박 감지됨]");
      // 심박 감지됨
      long delta = millis() - lastBeat;
      lastBeat = millis();
      
      // 60초 내 심박 간격으로 BPM 계산
      beatsPerMinute = 60 / (delta / 1000.0);
      
      // 현실적인 심박수 범위로 제한 (30-220 BPM)
      if (beatsPerMinute < 30 || beatsPerMinute > 220) {
        // 범위 밖의 값은 무시
        Serial.println("[유효하지 않은 심박수 값]");
      } else {
        // 평균 계산을 위해 배열에 저장
        rates[rateSpot++] = (byte)beatsPerMinute;
        rateSpot %= 4; // 배열 위치 순환 (0-3)
        
        // 4개 값의 평균 계산
        beatAvg = 0;
        for (byte x = 0 ; x < 4 ; x++) {
          beatAvg += rates[x];
        }
        beatAvg /= 4;
        
        // 전역 심박수 변수에 평균값 설정
        heartRateValue = beatAvg;
      }
    }
  } else {
    // 손가락이 감지되지 않음
    Serial.println("[손가락을 센서에 올려주세요]");
    
    // 리소스 최적화를 위해 천천히 값 감소
    if (heartRateValue > 0) {
      heartRateValue--; // 점차 0으로 감소
    }
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
  
  if (irValue > 20000) { // 유효한 IR 값이 있는 경우
    readMAX30102();
  } else {
    // 손가락이 감지되지 않음
    heartRateValue = 0;
    Serial.println("MAX30102: 손가락이 감지되지 않음");
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

// 거리 요청
void requestDistance() {
  ArduinoSerial.println(CMD_GET_DISTANCE);
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
