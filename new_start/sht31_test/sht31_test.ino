#include <Wire.h>
#include <Adafruit_SHT31.h>
#include <MAX30105.h>
#include <heartRate.h>

// SHT31 센서 객체 생성
Adafruit_SHT31 sht31 = Adafruit_SHT31();

// MAX30102 센서 객체 생성
MAX30105 particleSensor;

// I2C 핀 정의 (ESP32)
#define SDA_PIN 21
#define SCL_PIN 22

// 센서 주소 정의
#define SHT31_ADDR 0x44    // SHT31 기본 주소 (0x44 또는 0x45)
#define MAX30102_ADDR 0x57 // MAX30102 기본 주소
#define ARDUINO_ADDR 0x08  // 아두이노 주소

// 디버깅 설정
bool scanI2C = true;      // I2C 스캔 모드 활성화 여부
bool printInterval = true; // 센서 데이터 출력 간격 설정
unsigned long lastPrintTime = 0;
const int PRINT_INTERVAL = 2000; // 출력 간격 (ms)

// MAX30102 관련 변수
const byte RATE_SIZE = 4; // 평균을 위한 샘플 수 (메모리 때문에 작게 설정)
byte rates[RATE_SIZE];    // 심박수 배열
byte rateSpot = 0;
long lastBeat = 0;        // 마지막 심박 시간
float beatsPerMinute;

// 센서 상태 변수
bool sht31Ready = false;
bool max30102Ready = false;
bool arduinoConnected = false;

void setup() {
  // 시리얼 초기화
  Serial.begin(115200);
  while (!Serial) delay(10);  // Leonardo/Micro 호환성을 위한 대기
  
  Serial.println("\nESP32 I2C 센서 디버깅 코드 v1.0.2");
  Serial.println("- SHT31 온습도 센서");
  Serial.println("- MAX30102 심박/SpO2 센서");
  Serial.println("- 아두이노 연결 확인");
  Serial.println("------------------------------");
  
  // I2C 초기화
  Wire.begin(SDA_PIN, SCL_PIN);
  
  // I2C 스캔 모드가 활성화된 경우 I2C 장치 스캔 수행
  if (scanI2C) {
    scanI2CBus();
  }
  
  // SHT31 센서 초기화
  if (initSHT31()) {
    sht31Ready = true;
  } else {
    Serial.println("SHT31 센서 초기화 실패. 연결을 확인하세요.");
    Serial.println("프로그램은 계속 실행되지만 SHT31 데이터를 읽을 수 없습니다.");
  }
  
  // MAX30102 센서 초기화
  if (initMAX30102()) {
    max30102Ready = true;
  } else {
    Serial.println("MAX30102 센서 초기화 실패. 연결을 확인하세요.");
    Serial.println("프로그램은 계속 실행되지만 심박 데이터를 읽을 수 없습니다.");
  }
  
  // 아두이노 연결 확인
  if (checkArduinoConnection()) {
    arduinoConnected = true;
    Serial.println("아두이노 연결 확인 성공! (주소: 0x" + String(ARDUINO_ADDR, HEX) + ")");
    readArduinoStatus();
  } else {
    Serial.println("아두이노 연결 확인 실패. 연결을 확인하세요.");
    Serial.println("아두이노가 데이터를 반환하지 않습니다. 연결을 확인하세요.");
  }
  
  Serial.println("------------------------------");
  Serial.println("시작됨: 명령어는 'help'를 입력하세요.");
}

void loop() {
  // 현재 시간 가져오기
  unsigned long currentTime = millis();
  
  // 스캔 모드가 활성화된 경우 30초마다 I2C 버스 스캔
  static unsigned long lastScanTime = 0;
  if (scanI2C && (currentTime - lastScanTime > 30000)) {
    scanI2CBus();
    lastScanTime = currentTime;
  }
  
  // 센서 데이터 읽기 및 출력
  if (!printInterval || (currentTime - lastPrintTime > PRINT_INTERVAL)) {
    if (sht31Ready) {
      readSHT31Data();
    }
    
    if (max30102Ready) {
      readMAX30102Data();
    }
    
    lastPrintTime = currentTime;
  }
  
  // 심박 감지 처리 (루프마다 처리 필요)
  if (max30102Ready) {
    checkHeartbeat();
  }
  
  // 시리얼 명령어 처리
  handleSerialCommands();
  
  delay(10); // 약간의 딜레이 추가
}

// SHT31 센서 초기화 함수
bool initSHT31() {
  Serial.println("SHT31 센서 초기화 시도 중...");
  
  // 센서 주소를 이용한 초기화 시도
  if (sht31.begin(SHT31_ADDR)) {
    Serial.println("SHT31 센서 초기화 성공! (주소: 0x" + String(SHT31_ADDR, HEX) + ")");
    
    // 센서 히터 상태 확인
    Serial.print("SHT31 센서 히터 상태: ");
    if (sht31.isHeaterEnabled()) {
      Serial.println("켜짐");
    } else {
      Serial.println("꺼짐");
    }
    
    return true;
  }
  
  // 기본 주소로 초기화 실패 시, 대체 주소(0x45) 시도
  Serial.println("기본 주소(0x44)로 연결 실패, 대체 주소(0x45) 시도...");
  if (sht31.begin(0x45)) {
    Serial.println("SHT31 센서 초기화 성공! (주소: 0x45)");
    return true;
  }
  
  Serial.println("SHT31 센서 초기화 실패");
  return false;
}

// MAX30102 센서 초기화 함수
bool initMAX30102() {
  Serial.println("MAX30102 센서 초기화 시도 중...");
  
  // MAX30102 센서 초기화 시도
  if (particleSensor.begin(Wire, I2C_SPEED_FAST)) { // 400kHz로 설정
    Serial.println("MAX30102 센서 초기화 성공! (주소: 0x57)");
    
    // 센서 설정
    byte ledBrightness = 60; // 0=꺼짐, 255=50mA
    byte sampleAverage = 4;  // 1, 2, 4, 8, 16, 32
    byte ledMode = 2;       // 1=적색만, 2=적색+적외선
    byte sampleRate = 100;   // 50, 100, 200, 400, 800, 1000, 1600, 3200
    int pulseWidth = 411;    // 69, 118, 215, 411
    int adcRange = 4096;     // 2048, 4096, 8192, 16384
    
    particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);
    particleSensor.setPulseAmplitudeRed(0x0A); // 적색 LED 밝기 설정
    particleSensor.setPulseAmplitudeGreen(0);  // 녹색 LED 끄기 (MAX30102에 없음)
    
    // 센서 초기화 상태 확인
    if (particleSensor.getIR() > 0) {
      Serial.println("MAX30102 센서 동작 확인 완료");
      return true;
    } else {
      Serial.println("MAX30102 센서가 데이터를 반환하지 않습니다.");
      Serial.println("- 핀 연결을 확인하세요.");
      Serial.println("- 센서가 손상되었을 수 있습니다.");
      return false;
    }
  }
  
  Serial.println("MAX30102 센서 초기화 실패");
  Serial.println("- I2C 주소(0x57)를 확인하세요.");
  Serial.println("- 전원과 핀 연결을 확인하세요.");
  return false;
}

// 아두이노 연결 확인 함수
bool checkArduinoConnection() {
  Serial.println("아두이노 연결 확인 시도 중...");
  
  // 아두이노 주소를 이용한 초기화 시도
  Wire.beginTransmission(ARDUINO_ADDR);
  byte error = Wire.endTransmission();
  
  if (error == 0) {
    Serial.println("아두이노 연결 확인 성공! (주소: 0x" + String(ARDUINO_ADDR, HEX) + ")");
    return true;
  }
  
  Serial.println("아두이노 연결 확인 실패");
  return false;
}

// 아두이노 상태 읽기 함수
void readArduinoStatus() {
  Serial.println("아두이노 상태 읽기 시도 중...");
  
  // 아두이노 상태 읽기
  Wire.requestFrom(ARDUINO_ADDR, 1);
  byte status = Wire.read();
  
  Serial.print("아두이노 상태: ");
  Serial.println(status);
}

// SHT31 센서 데이터 읽기 함수
void readSHT31Data() {
  float temperature = sht31.readTemperature();
  float humidity = sht31.readHumidity();
  
  Serial.println("------------------------------");
  Serial.print("측정 시간: ");
  Serial.print(millis() / 1000);
  Serial.println(" 초");
  
  // 온도 데이터 확인 및 출력
  if (!isnan(temperature)) {
    Serial.print("온도: ");
    Serial.print(temperature);
    Serial.println(" °C");
  } else {
    Serial.println("온도 읽기 실패!");
    checkSensorStatus("SHT31");
    sht31Ready = false;
  }
  
  // 습도 데이터 확인 및 출력
  if (!isnan(humidity)) {
    Serial.print("습도: ");
    Serial.print(humidity);
    Serial.println(" %");
  } else {
    Serial.println("습도 읽기 실패!");
    checkSensorStatus("SHT31");
    sht31Ready = false;
  }
}

// MAX30102 센서 데이터 읽기 함수
void readMAX30102Data() {
  Serial.println("------------------------------");
  Serial.println("MAX30102 센서 데이터:");
  
  // 적외선 값과 적색 값 출력
  long irValue = particleSensor.getIR();
  long redValue = particleSensor.getRed();
  
  if (irValue < 5000) {
    Serial.println("손가락을 센서에 올려주세요.");
  } else {
    // 적외선 및 적색 값 출력
    Serial.print("IR 값: ");
    Serial.println(irValue);
    Serial.print("Red 값: ");
    Serial.println(redValue);
    
    // 심박수 출력
    if (beatsPerMinute > 30 && beatsPerMinute < 220) {
      Serial.print("❤️ 심박수: ");
      Serial.print(beatsPerMinute);
      Serial.println(" BPM");
    }
    
    // SpO2 추정값 계산 및 출력 (간단한 근사치 계산, 정확하지 않음)
    float ratio = (float)redValue / (float)irValue;
    float spo2 = 110.0 - 25.0 * ratio; // 매우 간단한 근사 계산법
    
    if (spo2 > 0 && spo2 <= 100) {
      Serial.print("SpO2 추정: ");
      Serial.print(spo2, 1);
      Serial.println("%");
      Serial.println("(주의: 정확한 의료기기가 아닙니다)");
    }
  }
}

// 심박 감지 처리 함수
void checkHeartbeat() {
  // 심박수 측정 알고리즘
  long irValue = particleSensor.getIR();
  
  if (irValue > 5000) {
    // 손가락이 감지됨
    if (checkForBeat(irValue)) {
      // 심박 감지됨
      long delta = millis() - lastBeat;
      lastBeat = millis();
      
      beatsPerMinute = 60 / (delta / 1000.0);
      
      // 비정상적인 값 필터링
      if (beatsPerMinute < 30 || beatsPerMinute > 220) {
        // 비정상적인 값 무시
        return;
      }
      
      // 평균 계산을 위해 배열에 저장
      rates[rateSpot++] = (byte)beatsPerMinute;
      rateSpot %= RATE_SIZE;
      
      // 이동 평균 계산
      beatsPerMinute = 0;
      for (byte x = 0; x < RATE_SIZE; x++)
        beatsPerMinute += rates[x];
      beatsPerMinute /= RATE_SIZE;
    }
  }
}

// 센서 상태 확인 함수
void checkSensorStatus(String sensorName) {
  Serial.println("센서 연결 또는 I2C 버스 확인 중...");
  
  // I2C 버스 확인
  Serial.print("I2C 버스 스캔 중... ");
  byte error, address;
  int deviceCount = 0;
  
  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    
    if (error == 0) {
      Serial.print("장치 발견: 0x");
      if (address < 16) Serial.print("0");
      Serial.print(address, HEX);
      Serial.print(" ");
      deviceCount++;
    }
  }
  
  if (deviceCount == 0) {
    Serial.println("I2C 장치를 찾을 수 없습니다!");
    Serial.println("- 연결 및 배선을 확인하세요");
    Serial.println("- SDA와 SCL 핀이 올바르게 연결되었는지 확인하세요");
    Serial.println("- 풀업 저항이 필요할 수 있습니다 (4.7K ohm)");
  } else {
    Serial.println();
    Serial.print(deviceCount);
    Serial.println(" 개의 I2C 장치를 발견했습니다.");
  }
  
  // 센서 재초기화 시도
  Serial.println(sensorName + " 센서 재초기화 시도...");
  
  if (sensorName == "SHT31") {
    if (initSHT31()) {
      Serial.println("SHT31 센서 재초기화 성공!");
      sht31Ready = true;
    } else {
      Serial.println("SHT31 센서 재초기화 실패. 하드웨어 연결을 확인하세요.");
      sht31Ready = false;
    }
  } else if (sensorName == "MAX30102") {
    if (initMAX30102()) {
      Serial.println("MAX30102 센서 재초기화 성공!");
      max30102Ready = true;
    } else {
      Serial.println("MAX30102 센서 재초기화 실패. 하드웨어 연결을 확인하세요.");
      max30102Ready = false;
    }
  }
}

// I2C 버스 스캔 함수
void scanI2CBus() {
  byte error, address;
  int deviceCount = 0;
  
  Serial.println("\nI2C 버스 스캔 중...");
  
  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    
    if (error == 0) {
      Serial.print("I2C 장치 발견: 0x");
      if (address < 16) Serial.print("0");
      Serial.print(address, HEX);
      
      // 센서 주소 확인
      if (address == 0x44 || address == 0x45) {
        Serial.print(" (SHT31 센서)");
      } else if (address == 0x57) {
        Serial.print(" (MAX30102 센서)");
      } else if (address == 0x08) {
        Serial.print(" (아두이노)");
      }
      
      Serial.println();
      deviceCount++;
    } else if (error == 4) {
      Serial.print("알 수 없는 오류 발생: 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
    }
  }
  
  if (deviceCount == 0) {
    Serial.println("I2C 장치를 찾을 수 없습니다!");
    Serial.println("가능한 문제 해결책:");
    Serial.println("1. SDA와 SCL 핀이 올바르게 연결되었는지 확인하세요");
    Serial.println("2. 외부 풀업 저항이 필요할 수 있습니다 (4.7K ohm)");
    Serial.println("3. ESP32의 다른 GPIO 핀으로 I2C를 시도해보세요");
    Serial.println("4. 센서 전원 공급이 안정적인지 확인하세요");
  } else {
    Serial.print("I2C 스캔 완료: ");
    Serial.print(deviceCount);
    Serial.println(" 개의 장치 발견");
    
    // 센서 존재 여부 확인
    bool foundSHT31 = false;
    bool foundMAX30102 = false;
    bool foundArduino = false;
    
    for (address = 1; address < 127; address++) {
      Wire.beginTransmission(address);
      error = Wire.endTransmission();
      
      if (error == 0) {
        if (address == 0x44 || address == 0x45) foundSHT31 = true;
        if (address == 0x57) foundMAX30102 = true;
        if (address == 0x08) foundArduino = true;
      }
    }
    
    // 센서 감지 결과 출력
    Serial.println("센서 감지 결과:");
    Serial.print("- SHT31 센서: ");
    Serial.println(foundSHT31 ? "발견됨" : "발견되지 않음");
    Serial.print("- MAX30102 센서: ");
    Serial.println(foundMAX30102 ? "발견됨" : "발견되지 않음");
    Serial.print("- 아두이노: ");
    Serial.println(foundArduino ? "발견됨" : "발견되지 않음");
    
    // 필요한 경우 센서 초기화 시도
    if (foundSHT31 && !sht31Ready) {
      if (initSHT31()) sht31Ready = true;
    }
    
    if (foundMAX30102 && !max30102Ready) {
      if (initMAX30102()) max30102Ready = true;
    }
    
    if (foundArduino && !arduinoConnected) {
      if (checkArduinoConnection()) arduinoConnected = true;
    }
  }
  Serial.println("------------------------------");
}

// 시리얼 명령어 처리 함수
void handleSerialCommands() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    
    if (command == "scan") {
      Serial.println("명령 실행: I2C 스캔");
      scanI2CBus();
    } 
    else if (command == "sht31") {
      Serial.println("명령 실행: SHT31 센서 읽기");
      if (sht31Ready) {
        readSHT31Data();
      } else {
        Serial.println("SHT31 센서가 준비되지 않았습니다.");
        if (initSHT31()) sht31Ready = true;
      }
    }
    else if (command == "max30102") {
      Serial.println("명령 실행: MAX30102 센서 읽기");
      if (max30102Ready) {
        readMAX30102Data();
      } else {
        Serial.println("MAX30102 센서가 준비되지 않았습니다.");
        if (initMAX30102()) max30102Ready = true;
      }
    }
    else if (command == "arduino") {
      Serial.println("명령 실행: 아두이노 상태 읽기");
      if (arduinoConnected) {
        readArduinoStatus();
      } else {
        Serial.println("아두이노가 준비되지 않았습니다.");
        if (checkArduinoConnection()) arduinoConnected = true;
      }
    }
    else if (command == "heater on") {
      Serial.println("명령 실행: SHT31 히터 켜기");
      if (sht31Ready) {
        sht31.heater(true);
        Serial.println("SHT31 히터 켜짐");
      } else {
        Serial.println("SHT31 센서가 준비되지 않았습니다.");
      }
    }
    else if (command == "heater off") {
      Serial.println("명령 실행: SHT31 히터 끄기");
      if (sht31Ready) {
        sht31.heater(false);
        Serial.println("SHT31 히터 꺼짐");
      } else {
        Serial.println("SHT31 센서가 준비되지 않았습니다.");
      }
    }
    else if (command == "status") {
      Serial.println("\n시스템 상태 정보:");
      Serial.print("- SHT31 센서: ");
      Serial.println(sht31Ready ? "준비됨" : "준비되지 않음");
      Serial.print("- MAX30102 센서: ");
      Serial.println(max30102Ready ? "준비됨" : "준비되지 않음");
      Serial.print("- 아두이노: ");
      Serial.println(arduinoConnected ? "준비됨" : "준비되지 않음");
      Serial.print("- I2C 스캔 모드: ");
      Serial.println(scanI2C ? "활성화됨" : "비활성화됨");
      Serial.print("- 주기적 출력: ");
      Serial.println(printInterval ? "활성화됨" : "비활성화됨");
      Serial.print("- 출력 간격: ");
      Serial.print(PRINT_INTERVAL);
      Serial.println(" ms");
    }
    else if (command == "interval on") {
      printInterval = true;
      Serial.println("주기적 출력 활성화됨");
    }
    else if (command == "interval off") {
      printInterval = false;
      Serial.println("주기적 출력 비활성화됨");
    }
    else if (command == "autoscan on") {
      scanI2C = true;
      Serial.println("자동 I2C 스캔 활성화됨");
    }
    else if (command == "autoscan off") {
      scanI2C = false;
      Serial.println("자동 I2C 스캔 비활성화됨");
    }
    else if (command == "reset") {
      Serial.println("시스템 리셋 중...");
      ESP.restart();
    }
    else if (command == "help") {
      Serial.println("\n사용 가능한 명령어:");
      Serial.println("  scan - I2C 버스 스캔");
      Serial.println("  sht31 - SHT31 온습도 센서 데이터 읽기");
      Serial.println("  max30102 - MAX30102 심박/SpO2 센서 데이터 읽기");
      Serial.println("  arduino - 아두이노 상태 읽기");
      Serial.println("  heater on - SHT31 히터 켜기");
      Serial.println("  heater off - SHT31 히터 끄기");
      Serial.println("  status - 시스템 상태 정보 표시");
      Serial.println("  interval on - 주기적 출력 활성화");
      Serial.println("  interval off - 주기적 출력 비활성화");
      Serial.println("  autoscan on - 자동 I2C 스캔 활성화");
      Serial.println("  autoscan off - 자동 I2C 스캔 비활성화");
      Serial.println("  reset - 시스템 재시작");
      Serial.println("  fan on/off - 팬 켜기/끄기");
      Serial.println("  led on/off - LED 켜기/끄기");
      Serial.println("  humidifier on/off - 가습기 켜기/끄기");
      Serial.println("  help - 도움말 표시");
    }
    else {
      Serial.println("알 수 없는 명령어: " + command);
      Serial.println("'help'를 입력하여 사용 가능한 명령어 확인");
    }
  }
}
