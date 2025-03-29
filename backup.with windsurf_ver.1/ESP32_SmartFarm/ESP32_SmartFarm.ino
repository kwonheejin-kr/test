#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include "Adafruit_SHT31.h"
#include "MAX30105.h"
#include "heartRate.h"
#include "spo2_algorithm.h"
#include "BluetoothSerial.h"
#include "ui.h"

// Pin Definitions
#define I2C_SDA 21
#define I2C_SCL 22
#define SD_CS 5
#define SLAVE_ADDR 0x08

// Audio Pins
#define I2S_BCLK 26
#define I2S_LRCLK 25
#define I2S_DIN 33

// Objects
Adafruit_SHT31 sht31;
MAX30105 particleSensor;
BluetoothSerial SerialBT;

// Variables for heart rate calculation
const byte RATE_SIZE = 4;
byte rates[RATE_SIZE];
byte rateSpot = 0;
long lastBeat = 0;
float beatsPerMinute;
int beatAvg;

// Variables for SpO2 calculation
uint32_t irBuffer[100];
uint32_t redBuffer[100];
int32_t spo2;
int8_t validSPO2;
int32_t heartRate;
int8_t validHeartRate;

void setup() {
    Serial.begin(115200);
    
    // I2C 초기화
    Wire.begin(I2C_SDA, I2C_SCL);
    
    // SHT31 센서 초기화
    if (!sht31.begin(0x44)) {
        Serial.println("SHT31 센서를 찾을 수 없습니다!");
    }
    
    // MAX30102 센서 초기화
    if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
        Serial.println("MAX30102 센서를 찾을 수 없습니다!");
    }
    particleSensor.setup(60, 4, 2, 411, 4096);
    
    // SD 카드 초기화
    if (!SD.begin(SD_CS)) {
        Serial.println("SD 카드 초기화 실패!");
    }
    
    // 블루투스 초기화
    SerialBT.begin("ESP32_SmartFarm");
    
    // UI 초기화
    ui_init();
}

void loop() {
    lv_timer_handler();
    delay(5);
}

// I2C를 통해 Arduino에 명령 전송
void sendCommand(char command) {
    Wire.beginTransmission(SLAVE_ADDR);
    Wire.write(command);
    Wire.endTransmission();
}

// Arduino로부터 상태 읽기
byte readStatus() {
    Wire.requestFrom(SLAVE_ADDR, 1);
    if (Wire.available()) {
        return Wire.read();
    }
    return 0;
}
