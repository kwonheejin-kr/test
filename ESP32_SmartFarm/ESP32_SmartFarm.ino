#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <SPI.h>

#include <Config.h>
#include <State.h>
#include <error_handler.h>
#include <system_monitor.h>
#include <comms.h>
#include <sd_manager.h>
#include <sensors.h>
#include <ui_manager.h>
#include <ui_events.h>

void setup() {
    Serial.begin(115200);
    Wire.begin();
    
    // SD 카드 초기화
    if (!sdManager.init(Config::SD_CS)) {
        handle_error("SD 카드 초기화 실패");
    }
    
    // 오디오 초기화
    if (!audioManager.init()) {
        handle_error("오디오 초기화 실패");
    }
    
    // 센서 초기화
    if (!sensorManager.init()) {
        handle_error("센서 초기화 실패");
    }
    
    // UI 초기화
    UIEventManager::init();
    
    // 시스템 모니터 초기화
    SystemMonitor::init();
}

void loop() {
    unsigned long currentMillis = millis();
    
    // 주기적인 작업 처리
    SystemMonitor::update();
    sensorManager.update();
    audioManager.handle_audio();
    
    // UI 이벤트 처리
    lv_timer_handler();
    
    delay(5);
}






