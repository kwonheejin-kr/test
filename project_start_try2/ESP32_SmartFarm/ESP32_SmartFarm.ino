#include "Wire.h"
#include "WiFi.h"
#include "SD.h"
#include "SPI.h"
#include "FS.h"
#include "driver/i2s.h"
#include "ui.h"
#include "ui_events.h"  // 🔹 UI 이벤트 헤더 파일 참조
#include "Adafruit_MAX30102.h"
#include "DHT.h"
#include "ESP_I2S.h"
#include "BluetoothA2DPSink.h"

#define SD_CS 13
#define DHT_PIN 4
#define DHT_TYPE DHT22
#define I2S_NUM I2S_NUM_0
#define I2S_BCK 5
#define I2S_LRCK 25
#define I2S_DOUT 26
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#define SERVER_IP "192.168.1.100"  // 라즈베리파이 IP

#define I2C_SLAVE_ADDR 0x08  // 아두이노 I2C 주소

Adafruit_MAX30102 heartRateSensor;
DHT dht(DHT_PIN, DHT_TYPE);

unsigned long lastSensorUpdate = 0;
unsigned long lastHeartRateUpdate = 0;
unsigned long lastAudioCheck = 0;
unsigned long lastFanRequest = 0;
unsigned long lastLedRequest = 0;
unsigned long lastSmogRequest = 0;

bool isPlaying = false;
File musicFile;

void setup() {
    Serial.begin(115200);
    Wire.begin();
    
    if (!SD.begin(SD_CS)) Serial.println("SD 카드 초기화 실패!");
    else Serial.println("SD 카드 초기화 성공.");

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("WiFi 연결 성공");

    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = 48000,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_24BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_I2S,
        .intr_alloc_flags = 0,
        .dma_buf_count = 16,
        .dma_buf_len = 128,
        .use_apll = true
    };

    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_BCK,
        .ws_io_num = I2S_LRCK,
        .data_out_num = I2S_DOUT,
        .data_in_num = I2S_PIN_NO_CHANGE
    };

    i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM, &pin_config);

    lv_init();
    ui_init();
}

void loop() {
    unsigned long currentMillis = millis();

    if (currentMillis - lastSensorUpdate >= 2000) {
        update_temp_hum();
        lastSensorUpdate = currentMillis;
    }

    if (currentMillis - lastHeartRateUpdate >= 500) {
        check_sensor_data();
        lastHeartRateUpdate = currentMillis;
    }

    if (currentMillis - lastAudioCheck >= 1000) {
        if (isPlaying) {
            play_audio();
        }
        lastAudioCheck = currentMillis;
    }

    // 🔹 릴레이 상태 개별 요청 (ui_events.c에서 정의된 함수 호출)
    if (currentMillis - lastFanRequest >= 100) {
        request_fan_status();
        lastFanRequest = currentMillis;
    }

    if (currentMillis - lastLedRequest >= 100) {
        request_led_status();
        lastLedRequest = currentMillis;
    }

    if (currentMillis - lastSmogRequest >= 100) {
        request_smog_status();
        lastSmogRequest = currentMillis;
    }

    lv_timer_handler();
    delay(5);
}
