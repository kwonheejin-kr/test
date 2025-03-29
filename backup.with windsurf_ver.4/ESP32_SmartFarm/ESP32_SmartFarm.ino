#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include "Adafruit_SHT31.h"
#include "MAX30105.h"
#include "heartRate.h"
#include "spo2_algorithm.h"
#include "BluetoothSerial.h"
#include "BluetoothA2DPSink.h"
#include "driver/i2s.h"
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
BluetoothA2DPSink a2dp_sink;

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
        Serial.println("SD ! ");
    } else {
        Serial.println("SD ! ");
        // SD 
        if (SD.exists(TOUCH_SOUND_PATH)) {
            Serial.println(": " + String(TOUCH_SOUND_PATH));
        } else {
            Serial.println(": " + String(TOUCH_SOUND_PATH));
        }
    }
    
    // 
    SerialBT.begin("ESP32_SmartFarm");
    Serial.println(" . . : ESP32_SmartFarm");
    
    // A2DP 
    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_BCLK,
        .ws_io_num = I2S_LRCLK,
        .data_out_num = I2S_DIN,
        .data_in_num = -1
    };
    
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = 44100,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 64,
        .use_apll = false,
        .tx_desc_auto_clear = true,
        .fixed_mclk = 0
    };
    
    a2dp_sink.set_i2s_port(I2S_NUM_0);
    a2dp_sink.set_i2s_config(i2s_config);
    a2dp_sink.start("ESP32_SmartFarm");
    
    // UI 
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
