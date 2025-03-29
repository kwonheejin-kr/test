#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include "Adafruit_SHT31.h"
#include "MAX30105.h"
#include "heartRate.h"
#include "spo2_algorithm.h"
#include "BluetoothSerial.h"
#include "ESP_I2S.h"
#include "BluetoothA2DPSink.h"
// #include "driver/i2s.h"  // ESP_I2S.h와 충돌하므로 제거
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
#define TOUCH_SOUND_PATH "/touch.wav"  // 

// Objects
Adafruit_SHT31 sht31;
MAX30105 particleSensor;
BluetoothSerial SerialBT;
I2SClass i2s;
BluetoothA2DPSink a2dp_sink(i2s);

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
    
    // I2C 
    Wire.begin(I2C_SDA, I2C_SCL);
    
    // SHT31 
    if (!sht31.begin(0x44)) {
        Serial.println("SHT31 ! ");
    }
    
    // MAX30102 
    if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
        Serial.println("MAX30102 ! ");
    }
    particleSensor.setup(60, 4, 2, 411, 4096);
    
    // SD 
    if (!SD.begin(SD_CS)) {
        Serial.println("SD card initialization failed!");
        // SD 카드 없어도 계속 진행
    } else {
        Serial.println("SD card initialized.");
        // 터치 사운드 파일 확인
        if (!SD.exists(TOUCH_SOUND_PATH)) {
            Serial.print("Touch sound file not found");
            Serial.println(": " + String(TOUCH_SOUND_PATH));
        } else {
            Serial.println(": " + String(TOUCH_SOUND_PATH));
        }
    }
    
    // 블루투스 시리얼 초기화
    SerialBT.begin("ESP32_SmartFarm");
    Serial.println("블루투스 시리얼 초기화: ESP32_SmartFarm");
    
    // I2S 설정
    i2s.setPins(I2S_BCLK, I2S_LRCLK, I2S_DIN);
    if (!i2s.begin(I2S_MODE_STD, 44100, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO, I2S_STD_SLOT_BOTH)) {
        Serial.println("❌ I2S 초기화 실패!");
        // 오류가 있어도 계속 진행 (다른 기능은 작동 가능)
    } else {
        Serial.println("✅ I2S 초기화 성공");
    }
    
    // 블루투스 오디오 시작
    a2dp_sink.start("ESP32_SmartFarm");
    Serial.println("✅ ESP32 Bluetooth Audio Streaming Started!");
    
    // UI 초기화
    ui_init();
}

void loop() {
    lv_timer_handler();
    delay(5);
}

// I2C 
void sendCommand(char command) {
    Wire.beginTransmission(SLAVE_ADDR);
    Wire.write(command);
    Wire.endTransmission();
}

// 
byte readStatus() {
    Wire.requestFrom(SLAVE_ADDR, 1);
    if (Wire.available()) {
        return Wire.read();
    }
    return 0;
}
