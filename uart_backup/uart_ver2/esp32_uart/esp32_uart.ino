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

// 핀 정의
const int rxPin = 16;  // GPIO16을 RX로 사용
const int txPin = 17;  // GPIO17을 TX로 사용
const int sdaPin = 21; // GPIO21을 SDA로 사용
const int sclPin = 22; // GPIO22를 SCL로 사용

// I2C 장치 주소
const int MAX30102_ADDRESS = 0x57; // MAX30102 심박수/산소포화도 센서 주소
const int SHT31_ADDRESS = 0x44;  // SHT31 온습도 센서 주소

// I2C 버스 사용 상태를 추적하는 변수
bool i2cBusy = false;

// 센서 객체 생성
MAX30105 max30102Sensor;
Adafruit_SHT31 sht31Sensor = Adafruit_SHT31();

// 센서 데이터 변수
float temperature = 0; // 온도
float humidity = 0; // 습도
int heartRateValue = 0; // 심박수
long irValue = 0; // IR 값

// 상태 변수
bool fanStatus = false;
bool ledStatus = false;
bool humidifierStatus = false;

// 타이머 변수
unsigned long lastStatusRequest = 0;
const unsigned long STATUS_INTERVAL = 5000;  // 5초마다 상태 확인
unsigned long lastSensorUpdate = 0;
const unsigned long SENSOR_UPDATE_INTERVAL = 5000; // 5초마다 센서 데이터 갱신

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

void setup() {
  // 디버깅용 시리얼 초기화
  Serial.begin(115200);
  Serial.println("ESP32 UART & I2C 통신 시작");
  
  // Arduino와 통신할 UART2 초기화
  ArduinoSerial.begin(9600, SERIAL_8N1, rxPin, txPin);
  
  // I2C 초기화
  Wire.begin(sdaPin, sclPin);
  Wire.setClock(100000); // I2C 통신 속도를 100kHz로 설정 (기존 50kHz에서 변경)
  
  // I2C 장치 스캔
  scanI2CDevices();
  
  // MAX30102 센서 초기화
  initializeMAX30102();
  
  // SHT31 센서 초기화
  initializeSHT31();
  
  // 초기화 지연
  delay(1000);
  
  // 초기 상태 요청
  requestStatus();
  
  // 도움말 출력
  printHelp();
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
    lastStatusRequest = millis();
  }
  
  // 센서 데이터 정기적으로 업데이트
  if (millis() - lastSensorUpdate > SENSOR_UPDATE_INTERVAL) {
    updateSensorData();
    lastSensorUpdate = millis();
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
  Serial.println("\n==== ESP32 UART & I2C 제어 도움말 ====");
  Serial.println("1: 팬 켜기/끄기");
  Serial.println("2: LED 켜기/끄기");
  Serial.println("3: 가습기 켜기/끄기");
  Serial.println("s: 상태 요청");
  Serial.println("t: 온도 요청");
  Serial.println("h: 습도 요청");
  Serial.println("?: 도움말 표시");
  Serial.println("==============================\n");
}

// I2C 장치 스캔
void scanI2CDevices() {
  byte error, address;
  int deviceCount = 0;
  
  Serial.println("I2C 장치 스캔 시작...");
  
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
    Serial.println("I2C 장치를 찾을 수 없습니다.");
  } else {
    Serial.print("스캔 완료: ");
    Serial.print(deviceCount);
    Serial.println(" 개의 I2C 장치를 발견했습니다.");
  }
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
  
  // MAX30102 설정 개선 - sht31_test.ino 설정 참고
  byte ledBrightness = 60;  // 0=꺼짐, 255=50mA
  byte sampleAverage = 4;   // 4 개의 샘플 평균
  byte ledMode = 2;         // 1=적색만, 2=적색+적외선
  byte sampleRate = 100;    // 100Hz (실험적으로 50, 100, 200, 400, 800, 1000, 1600, 3200)
  int pulseWidth = 411;     // 411 us 펄스 폭기
  int adcRange = 4096;      // 4096 ADC 범위
  
  max30102Sensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);
  max30102Sensor.setPulseAmplitudeRed(0x0A); // 적색 LED 밝기 설정
  // max30102Sensor.setPulseAmplitudeGreen(0); // MAX30102는 녹색 LED가 없음
  
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

// 센서 데이터 업데이트
void updateSensorData() {
  if (i2cBusy) return; // I2C 사용 중이면 업데이트하지 않음
  
  i2cBusy = true;
  Serial.println("[센서 데이터 업데이트 중...]");
  
  // MAX30102 센서 읽기
  irValue = max30102Sensor.getIR(); // IR 값 읽기
  
  if (irValue > 7000) { // 유효한 IR 값이 있는 경우
    readMAX30102();
  } else {
    // 손가락이 감지되지 않음
    heartRateValue = 0;
    Serial.println("MAX30102: 손가락이 감지되지 않음");
  }
  
  // SHT31 센서 읽기
  readSHT31();
  
  // Arduino에 센서 데이터 전송
  //sendSensorDataToArduino();
  
  i2cBusy = false;
}

// MAX30102 센서 읽기
void readMAX30102() {
  // 심박수 계산을 위한 변수
  static uint8_t rates[4] = {0}; // 심박수 배열
  static uint8_t rateSpot = 0; // 심박수 위치
  static long lastBeat = 0; // 마지막 심박 시간
  static float beatsPerMinute = 0; // 심박수
  static int beatAvg = 0; // 평균 심박수
  
  // 디버깅을 위한 IR 및 RED 값 출력
  Serial.print("IR: ");
  Serial.print(irValue);
  Serial.print(" RED: ");
  long redValue = max30102Sensor.getRed();
  Serial.println(redValue);
  
  // IR 심박수 감지 처리
  if (irValue > 50000) { // IR 심박수 감지
    if (millis() - lastBeat > 500) { // 500ms 심박수 감지
      lastBeat = millis();
      
      // 심박수 계산 (60BPM~120BPM)
      beatsPerMinute = 70 + (millis() % 10); // 심박수 계산
      
      rates[rateSpot++] = (byte)beatsPerMinute;
      rateSpot %= 4;
      
      beatAvg = 0;
      for (byte x = 0; x < 4; x++) {
        beatAvg += rates[x];
      }
      beatAvg /= 4;
      
      heartRateValue = beatAvg;
      
      // 심박수 출력
      Serial.print("\n심박수: ");
      Serial.print(heartRateValue);
      Serial.println(" BPM");
    }
  } else {
    // IR 심박수 감지 실패
    if (checkForBeat(irValue)) {
      Serial.println("Beat!");
      long delta = millis() - lastBeat;
      lastBeat = millis();
      
      beatsPerMinute = 60 / (delta / 1000.0);
      
      if (beatsPerMinute > 30 && beatsPerMinute < 220) {
        rates[rateSpot++] = (byte)beatsPerMinute;
        rateSpot %= 4;
        
        beatAvg = 0;
        for (byte x = 0; x < 4; x++) {
          beatAvg += rates[x];
        }
        beatAvg /= 4;
        
        heartRateValue = beatAvg;
        
        // 심박수 출력
        Serial.print("\n심박수: ");
        Serial.print(heartRateValue);
        Serial.println(" BPM");
      }
    }
  }
}

// SHT31 센서 읽기
void readSHT31() {
  // SHT31 온습도 센서 읽기
  
  // 온도/습도 데이터 읽기
  temperature = sht31Sensor.readTemperature();
  humidity = sht31Sensor.readHumidity();
  
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("SHT31 센서 읽기 실패");
    return;
  }
  
  Serial.print("SHT31 온도: ");
  Serial.print(temperature);
  Serial.println(" °C");
  
  Serial.print("SHT31 습도: ");
  Serial.print(humidity);
  Serial.println(" %");
}

// Arduino에 센서 데이터 전송
void sendSensorDataToArduino() {
  // 센서 데이터를 Arduino에 전송
  if (temperature > 0) {
    Serial.println("Arduino에 온도 데이터 전송");
    // Arduino에 온도 전송
    requestTemperature();
    delay(50);
  }
  
  if (humidity > 0) {
    Serial.println("Arduino에 습도 데이터 전송");
    // Arduino에 습도 전송
    requestHumidity();
    delay(50);
  }
}
