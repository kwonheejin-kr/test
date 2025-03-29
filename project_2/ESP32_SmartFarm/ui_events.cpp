//주의 사항*
//humidifier은 label ui, smog는 event 이름으로 혼동의 요소가 강함


// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.5.0
// LVGL version: 9.1.0
// Project name: SquareLine_Project_change

// C++ 헤더 먼저 포함
#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include "Adafruit_SHT31.h"
#include "MAX30105.h"
// #include "heartRate.h" - 직접 구현으로 대체
#include "spo2_algorithm.h"
#include "BluetoothSerial.h"
// ESP_I2S.h와 driver/i2s.h의 충돌 방지를 위해 하나만 포함
#include "ESP_I2S.h"
#include "BluetoothA2DPSink.h"
#include "ui.h"

// 그 다음 C 헤더 포함
extern "C" {
#include "ui_events.h"
#include "ui_helpers.h"
#include "lvgl.h"
// driver/i2s.h는 ESP_I2S.h로 대체
// #include "driver/i2s.h"
}

// 모든 함수는 extern "C"로 선언하여 C에서 호출할 수 있도록 함
extern "C" {

#define SD_CS 5  // SD 카드 칩 셀렉트 핀
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#define SERVER_IP "192.168.1.100"  // 라즈베리파이 IP
#define I2C_SDA 21
#define I2C_SCL 22
#define SLAVE_ADDR 0x08  // Arduino 슬레이브 주소

// Audio Pins
#define I2S_BCLK 26
#define I2S_LRCLK 25
#define I2S_DIN 33
#define TOUCH_SOUND_PATH "/touch.wav"

// 블루투스 객체 참조 추가
extern Adafruit_SHT31 sht31;
extern MAX30105 particleSensor;
extern BluetoothSerial SerialBT; //블루투스 객체

// UI 객체 정의
lv_obj_t *ui_smog; // ui_smog 객체 정의
lv_obj_t * ui_StartBtn;
lv_obj_t * ui_ProgressBar;
lv_obj_t * ui_ProgressLabel;
lv_obj_t * ui_HeartResult; // 추가된 변수

bool is_measuring = false;
unsigned long start_time = 0;
File heartDataFile;
File musicFile;

//라즈베리파이에 데이터 전송하는 내용



//블루투스 연결 상태 확인용 타이머 핸들러

lv_timer_t * bt_timer= NULL;
unsigned long bt_start_time = 0; //bluetooth for millis

//fan toggle millis 구조

unsigned long fan_last_request = 0;
bool fan_request_pending = false;

//led toggle millis

unsigned long led_last_request = 0;
bool led_request_pending = false;

//smog toggle millis
unsigned long smog_last_request = 0;
bool smog_request_pending = false;

void request_fan_status(lv_timer_t * timer) {
    Wire.beginTransmission(SLAVE_ADDR);
    Wire.write('f'); // 팬 상태 요청 명령 전송
    Wire.endTransmission();
    
    delay(50); // 응답 대기
    
    char fan_state = '0';
    if (Wire.requestFrom(SLAVE_ADDR, 1)) {
        if (Wire.available()) {
            fan_state = Wire.read();
        }
    }
    
    if (fan_state == '1' || fan_state == '0') {
        lv_label_set_text(ui_fan, (fan_state == '1') ? "Fan ON" : "Fan OFF");
    }
}

void fan_toggle(lv_event_t * e)
{
    Serial.println("팬 토글 명령 전송...");
    Wire.beginTransmission(SLAVE_ADDR);
    Wire.write('T');
    Wire.endTransmission();

    fan_last_request = millis();  
    fan_request_pending = true;

    // 상태 확인 요청을 일정 시간 뒤에 수행하도록 타이머 추가
    lv_timer_create(request_fan_status, 300, NULL);  // 300ms 후 상태 확인
}


void request_smog_status(lv_timer_t * timer) {
    Wire.beginTransmission(SLAVE_ADDR);
    Wire.write('h'); // 가습기 상태 요청 명령 전송
    Wire.endTransmission();
    
    delay(50); // 응답 대기
    
    char smog_state = '0';
    if (Wire.requestFrom(SLAVE_ADDR, 1)) {
        if (Wire.available()) {
            smog_state = Wire.read();
        }
    }
    
    if (smog_state == '1' || smog_state == '0') {
        lv_label_set_text(ui_smog, (smog_state == '1') ? "Humidifier ON" : "Humidifier OFF");
    }
}

void toggle_smog(lv_event_t * e) {
    Serial.println("가습기 토글 명령 전송...");
    Wire.beginTransmission(SLAVE_ADDR);
    Wire.write('H');  // 가습기 토글 명령 전송
    Wire.endTransmission();

    smog_last_request = millis();  
    smog_request_pending = true;

    // 상태 확인 타이머 시작 (없으면 생성)
    lv_timer_create(request_smog_status, 300, NULL);  // 300ms 후 상태 확인
}

void request_led_status(lv_timer_t * timer) {
    Wire.beginTransmission(SLAVE_ADDR);
    Wire.write('l'); // LED 상태 요청 명령 전송
    Wire.endTransmission();
    
    delay(50); // 응답 대기
    
    char led_state = '0';
    if (Wire.requestFrom(SLAVE_ADDR, 1)) {
        if (Wire.available()) {
            led_state = Wire.read();
        }
    }
    
    if (led_state == '1' || led_state == '0') {
        lv_label_set_text(ui_led, (led_state == '1') ? "LED ON" : "LED OFF");
    }
}


void toggle_led(lv_event_t * e)
{
	Serial.println("LED 토글 명령 전송...");
    Wire.beginTransmission(SLAVE_ADDR);
    Wire.write('L');  // 'L' (LED Toggle) 명령 전송
    Wire.endTransmission();
    
    led_last_request = millis();  // 마지막 요청 시간 기록
    led_request_pending = true;  // LED 상태 요청 예약
}


void send_data_to_raspberry() {
    // WiFi 연결 확인
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi not connected");
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
        delay(5000);  // WiFi 연결 대기
    }
    
    WiFiClient client;
    if (!client.connect(SERVER_IP, 5000)) {
        Serial.println("Connection to server failed");
        return;
    }
    
    // SD 카드에서 데이터 파일 열기
    File dataFile = SD.open("/heartDATA/heart_data.txt", FILE_READ);
    if (!dataFile) {
        Serial.println("Failed to open data file");
        client.stop();
        return;
    }
    
    // 데이터 전송
    uint8_t buffer[1024];
    while (dataFile.available()) {
        int bytesRead = dataFile.read(buffer, sizeof(buffer));
        client.write(buffer, bytesRead);  // 데이터 패킷 단위 전송
    }
    
    dataFile.close();
    client.stop();
    Serial.println("Data sent to Raspberry Pi");
}

void save_heart_data(float bpm) {
    if (!SD.exists("/heartDATA")) {
        SD.mkdir("/heartDATA");
    }
    heartDataFile = SD.open("/heartDATA/heart_data.txt", FILE_APPEND);
    if (heartDataFile) {
        heartDataFile.print(millis());
        heartDataFile.print(", ");
        heartDataFile.println(bpm);
        heartDataFile.close();
    }
}

void update_Sensor_Data(lv_event_t * e) {
    // 센서 데이터 업데이트 로직
    // 온습도 센서 데이터 읽기
    float temp = sht31.readTemperature();
    float hum = sht31.readHumidity();
    
    // 데이터가 유효한지 확인
    if (!isnan(temp) && !isnan(hum)) {
        // UI 업데이트 로직 구현
        char tempStr[10];
        char humStr[10];
        sprintf(tempStr, "%.1f°C", temp);
        sprintf(humStr, "%.1f%%", hum);
        
        // UI 요소 업데이트 (label에 표시) - 변수명 수정
        lv_label_set_text(ui_tempData, tempStr);
        lv_label_set_text(ui_humData, humStr);
    }
}

void HeartBeatMeasuringStart(lv_event_t * e) {
    // 심박 측정 시작 로직
    Serial.println("Heart beat measuring started");
    
    // 측정 진행 타이머 설정
    lv_timer_t * timer = lv_timer_create(measure_heart_rate, 100, NULL);
    lv_timer_set_repeat_count(timer, 50);  // 5초 동안 측정 (100ms * 50)
    
    // 측정 완료 후 데이터 체크 타이머 설정
    lv_timer_t * check_timer = lv_timer_create(check_sensor_data, 5000, NULL);
    lv_timer_set_repeat_count(check_timer, 1);  // 한 번만 실행
}

void start_bluetooth_connection(lv_event_t * e) {
    // 블루투스 연결 시작
    Serial.println("Bluetooth connection started");
    
    // 블루투스 이름 설정
    SerialBT.begin("ESP32_SmartFarm");
    
    // 블루투스 상태 모니터링 타이머 설정
    lv_timer_create(start_bluetooth_status, 1000, NULL);  // 1초마다 상태 체크
    
    // UI 업데이트 - 변수명 수정
    lv_label_set_text(ui_bluetoothLabel, "연결 대기 중...");
}

void measure_heart_rate(lv_timer_t * timer) {
    static int progress = 0;
    progress += 2; // 2% 씩 증가
    
    if (progress <= 100) {
        // 진행 상태 업데이트
        lv_bar_set_value(ui_ProgressBar, progress, LV_ANIM_ON);
        char progressText[10];
        sprintf(progressText, "%d%%", progress);
        lv_label_set_text(ui_ProgressLabel, progressText);
        
        // 심박수 센서 데이터 읽기
        particleSensor.check();
    }
}

void check_sensor_data(lv_timer_t * timer) {
    // 심박수 및 산소포화도 계산
    int32_t heartRate = random(65, 85); // 테스트용 랜덤 값 (실제로는 센서에서 계산)
    int32_t spo2 = random(95, 100);     // 테스트용 랜덤 값
    
    // 결과 표시 준비
    char resultText[50];
    sprintf(resultText, "심박수: %ld BPM\nSpO2: %ld%%", heartRate, spo2);
    
    // 결과 라벨에 표시
    lv_label_set_text(ui_HeartResult, resultText);
    
    // SD 카드에 데이터 저장
    save_heart_data(heartRate);
}

void start_bluetooth_status(lv_timer_t * timer) {
    // 블루투스 연결 상태 확인
    if (SerialBT.connected()) {
        // 연결됨 상태 표시
        lv_label_set_text(ui_bluetoothLabel, "연결됨");
    } else {
        // 연결 대기 중 상태 표시
        lv_label_set_text(ui_bluetoothLabel, "연결 대기 중...");
    }
}

} // extern "C" 블록 닫기
