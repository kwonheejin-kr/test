/*
 * ESP32 SmartFarm 
 * : 1.0.9
 */
#define LV_CONF_INCLUDE_SIMPLE 1

// 
#include <Arduino.h>

// ESP32 
#ifdef ARDUINO_ARCH_ESP32
  #include <esp32-hal.h>
#endif

// TFT_eSPI 
#include <TFT_eSPI.h>

// 
#include <Wire.h>
#include <SPI.h>

// 
#include <SD.h>
#include <BluetoothSerial.h>

// 
#include "Adafruit_SHT31.h"
#include "MAX30105.h"
#include "heartRate.h"
#include "spo2_algorithm.h"
#include "ESP_I2S.h"
#include "BluetoothA2DPSink.h"
// #include "driver/i2s.h"  // ESP_I2S.h 
#include "ui.h"

// Pin Definitions
#define I2C_SDA 21
#define I2C_SCL 22
#define SD_CS 5
#define SLAVE_ADDR 0x08

// TFT LCD Pin Definitions
#define TFT_MOSI 13
#define TFT_MISO 12
#define TFT_SCK 14
#define TFT_CS 15
#define TFT_DC 2
#define TFT_RST 4
#define TFT_BL 17  // 백라이트 핀 (GPIO17 사용)
#define TOUCH_CS 27
#define TOUCH_IRQ 32

// Audio Pins
#define I2S_BCLK 26
#define I2S_LRCLK 25
#define I2S_DIN 33
#define TOUCH_SOUND_PATH "/touch.wav"  

// LVGL 
#define LVGL_BUFFER_SIZE ((LV_HOR_RES_MAX * LV_VER_RES_MAX) / 25) // 버퍼 크기 조정 (1/10 -> 1/25) DRAM 사용량 감소

// Objects
Adafruit_SHT31 sht31;
MAX30105 particleSensor;
BluetoothSerial SerialBT;
I2SClass i2s;
BluetoothA2DPSink a2dp_sink(i2s);
TFT_eSPI tft = TFT_eSPI(); // TFT_eSPI 

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

// LVGL v9.2.2 
static lv_display_t *display = NULL;
static lv_indev_t *indev_touchpad = NULL;

// 정적 버퍼 사용 (동적 할당 제거)
static lv_color_t disp_buf1[LVGL_BUFFER_SIZE];
static lv_color_t disp_buf2[LVGL_BUFFER_SIZE];

// 
static void disp_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    uint32_t w = lv_area_get_width(area);
    uint32_t h = lv_area_get_height(area);

    // 디버깅 정보 출력 (문제 진단용)
    Serial.printf("Flushing area: x1=%d, y1=%d, w=%d, h=%d\n", area->x1, area->y1, w, h);

    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushPixels((uint16_t *)px_map, w * h);
    tft.endWrite();

    lv_display_flush_ready(disp);
}

// 
static void touchpad_read_cb(lv_indev_t *indev, lv_indev_data_t *data)
{
    uint16_t touchX = 0, touchY = 0;
    bool touched = false;
    
    #ifdef TOUCH_CS
    // TFT_eSPI 
    touched = tft.getTouch(&touchX, &touchY);
    #endif

    if (touched) {
        data->state = LV_INDEV_STATE_PRESSED;
        data->point.x = touchX;
        data->point.y = touchY;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

unsigned long lastReadTime = 0;

// LCD와 백라이트가 제대로 작동하는지 테스트하는 함수
void testLCD() {
  Serial.println("LCD 테스트 시작...");
  
  // 화면 전체를 검은색으로 지우기
  tft.fillScreen(TFT_BLACK);
  delay(500);
  
  // 간단한 도형 그리기 테스트
  tft.drawRect(10, 10, tft.width()-20, tft.height()-20, TFT_WHITE);
  delay(500);
  
  // 빨간색으로 채우기
  tft.fillScreen(TFT_RED);
  Serial.println("빨간색 화면 표시");
  delay(1000);
  
  // 녹색으로 채우기
  tft.fillScreen(TFT_GREEN);
  Serial.println("녹색 화면 표시");
  delay(1000);
  
  // 파란색으로 채우기
  tft.fillScreen(TFT_BLUE);
  Serial.println("파란색 화면 표시");
  delay(1000);
  
  // 테스트용 텍스트 출력
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(20, 100);
  tft.println("LCD 테스트 성공!");
  Serial.println("LCD 테스트 완료");
  delay(2000);
}

void setup() {
  // Serial 
  Serial.begin(115200);
  Serial.println("ESP32 SmartFarm v1.0.9 시작");

  // I2C 초기화 (SDA, SCL)
  Wire.begin(I2C_SDA, I2C_SCL);

  // TFT LCD 초기화
  tft.begin();
  SPI.setFrequency(20000000); // SPI 주파수 조정 (화면 깨짐 방지)
  tft.setRotation(0); // 화면 회전 설정
  tft.fillScreen(TFT_BLACK);
  
  // 백라이트 핀 초기화
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);  // 백라이트 켜기
  Serial.println("TFT LCD 디스플레이 초기화 완료 (백라이트 GPIO17 사용)");
  
  // LCD 기본 테스트 실행
  testLCD();
  
  // LVGL 초기화
  lv_init();
  Serial.println("LVGL 초기화 완료");
  
  // TFT 설정 확인
  Serial.printf("TFT 설정: 너비=%d, 높이=%d\n", tft.width(), tft.height());
  
  // LVGL v9.2.2 디스플레이 설정
  display = lv_display_create(tft.width(), tft.height());
  if (!display) {
    Serial.println("LVGL 디스플레이 생성 실패!");
    return;
  }
  lv_display_set_flush_cb(display, disp_flush_cb);
  
  // 버퍼 정보 출력
  Serial.printf("디스플레이 버퍼 크기: %d 바이트\n", sizeof(disp_buf1));
  
  // 디스플레이 버퍼 설정 - 정적 버퍼 사용, 직접 렌더링 모드로 변경
  lv_display_set_buffers(display, disp_buf1, disp_buf2, sizeof(disp_buf1), LV_DISPLAY_RENDER_MODE_DIRECT);
  Serial.println("LVGL 디스플레이 설정 완료");
  
  // 터치 입력 설정
  indev_touchpad = lv_indev_create();
  lv_indev_set_type(indev_touchpad, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev_touchpad, touchpad_read_cb);
  lv_indev_set_display(indev_touchpad, display);
  Serial.println("LVGL 터치 입력 설정 완료");
  
  // 기본 LVGL 테스트 레이블 생성 (UI 초기화 전 기본 테스트)
  lv_obj_t * label = lv_label_create(lv_screen_active());
  lv_label_set_text(label, "LCD 테스트 중...");
  lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
  lv_task_handler(); // 화면 업데이트 강제 실행
  delay(1000);
  
  // LVGL 기본 테스트 화면 - 최소한의 테스트만 실행
  label = lv_label_create(lv_screen_active());
  lv_label_set_text(label, "ESP32 SmartFarm");
  lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
  Serial.println("LVGL 테스트 레이블 생성 완료");
  
  // 화면 갱신
  lv_timer_handler();
  delay(500);
  
  // SHT31 초기화
  if (!sht31.begin(0x44)) {
    Serial.println("SHT31 센서를 찾을 수 없습니다!");
  } else {
    Serial.println("SHT31 센서 초기화 완료");
  }
  
  // MAX30102 초기화
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("MAX30102 센서를 찾을 수 없습니다!");
  } else {
    particleSensor.setup(60, 4, 2, 411, 4096);
    Serial.println("MAX30102 센서 초기화 완료");
  }
  
  // SD 카드 초기화
  if (!SD.begin(SD_CS)) {
    Serial.println("SD 카드 초기화 실패!");
  } else {
    Serial.println("SD 카드 초기화 완료");
    // 터치 사운드 파일 확인
    if (!SD.exists(TOUCH_SOUND_PATH)) {
      Serial.print("터치 사운드 파일을 찾을 수 없습니다");
      Serial.println(": " + String(TOUCH_SOUND_PATH));
    } else {
      Serial.println("터치 사운드 파일 발견: " + String(TOUCH_SOUND_PATH));
    }
  }
  
  // 블루투스 초기화
  SerialBT.begin("ESP32_SmartFarm");
  Serial.println("블루투스 시리얼 초기화 완료: ESP32_SmartFarm");
  
  // I2S 오디오 초기화
  i2s.setPins(I2S_BCLK, I2S_LRCLK, I2S_DIN);
  if (!i2s.begin(I2S_MODE_STD, 44100, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO, I2S_STD_SLOT_BOTH)) {
    Serial.println("I2S 초기화 실패");
  } else {
    Serial.println("I2S 초기화 완료");
  }
  
  // 블루투스 오디오 스트리밍 시작
  a2dp_sink.start("ESP32_SmartFarm");
  Serial.println("블루투스 오디오 스트리밍 시작!");
  
  Serial.println("ESP32 SmartFarm 초기화 완료");
}

void loop() {
  // LVGL - UI 업데이트
  lv_timer_handler();
  
  // 온습도 센서 데이터 읽기 (1초마다)
  if (millis() - lastReadTime > 1000) { // 1초 간격으로 읽기
    // SHT31 데이터 읽기
    float temperature = sht31.readTemperature();
    float humidity = sht31.readHumidity();
    
    // 유효한 데이터가 있으면 처리
    if (!isnan(temperature) && !isnan(humidity)) {
      Serial.print("온도: "); Serial.print(temperature); Serial.print("°C, 습도: "); Serial.print(humidity); Serial.println("%");
      // TODO: UI 업데이트 (온습도 표시)
      
      // 릴레이 상태 읽기
      readStatus();
    }
    
    lastReadTime = millis();
  }
  
  delay(10); // CPU 부하 방지 (5 → 10ms)
}

// I2C 명령 전송
void sendCommand(char command) {
  Wire.beginTransmission(SLAVE_ADDR);
  Wire.write(command);
  Wire.endTransmission();
}

// 릴레이 상태 읽기
byte readStatus() {
  Wire.requestFrom(SLAVE_ADDR, 1);
  if (Wire.available()) {
    return Wire.read();
  }
  return 0;
}
