#include <Adafruit_SH110X.h>
#include <splash.h>

// 1. C++ 표준 라이브러리
#include <Arduino.h>

// 2. 통신 관련 라이브러리
#include <Wire.h>
#include <WiFi.h>
#include <SPI.h>

// 3. 파일 시스템 라이브러리
#include <SD.h>
#include "FS.h"

// 4. 오디오 관련 라이브러리
#include "driver/i2s.h"
#include "AudioFileSourceSD.h"
#include "AudioGeneratorWAV.h"
#include "AudioOutputI2S.h"

// 5. 센서 라이브러리
#include "Adafruit_SHT31.h"

// 6. 프로젝트 설정 및 상태
#include "Config.h"
#include "State.h"

// 7. UI 관련
#include "lvgl.h"
#include "ui_manager.h"
#include "ui_events.h"

// 8. 프로젝트 컴포넌트
#include "error_handler.h"
#include "sensors.h"
#include "wifi_manager.h"
#include "comms.h"
#include "sd_utils.h"
#include "system_monitor.h"
#include "BluetoothA2DPSink.h"

// 오디오 객체 선언 제거
// AudioGeneratorWAV *wav = nullptr;
// AudioFileSourceSD *file = nullptr;
// AudioOutputI2S *out = nullptr;

// 전역 객체 선언은 유지
extern SDManager sdManager;

void setup() {
    // 시리얼 초기화
    Serial.begin(115200);

    // I2C 초기화
    Wire.begin();
    
    // SD 카드 초기화
    if (!sdManager.init(Config::SD_CS)) {
        handle_error("SD 카드 초기화 실패");
    } else {
        State::isSdCardInitialized = true;
    }

    // 센서 초기화
    if (!sensorManager.init()) {
        handle_error("센서 초기화 실패");
    }
    
    // UI 초기화
    initUI();
    
    // UI 이벤트 매니저 초기화
    UIEventManager::init();

    // 시스템 모니터링 초기화
    SystemMonitor::init();

    // Bluetooth 초기화
    if (!commManager.initBluetooth()) {
        handle_error("Bluetooth 초기화 실패");
    }
}

void loop() {
    unsigned long currentMillis = millis();
    
    // 타이머 업데이트
    Timers::update(currentMillis);

    // 오디오 처리
    sdManager.handle_audio();

    delay(5);
}

void playAudio(const char* filename) {
    if (State::isAudioPlaying || !State::isSdCardInitialized) return;
    sdManager.play_sound(filename);
}














