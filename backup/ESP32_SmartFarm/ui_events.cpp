// 1. UI 관련 헤더
#include "ui_events.h"
#include "State.h"
#include "sd_utils.h"
#include "sensors.h"

// 2. 프로젝트 컴포넌트
#include "comms.h"
#include "actuator_control.h"
#include "system_monitor.h"

void UIEventManager::init() {
    // 주기적인 센서 데이터 업데이트 타이머 설정
    lv_timer_create(update_sensor_readings, SENSOR_UPDATE_INTERVAL, NULL);
    
    // 릴레이 상태 체크 타이머 설정
    lv_timer_create(update_relay_status, RELAY_CHECK_INTERVAL, NULL);
}

void UIEventManager::handle_heart_measure_start(lv_event_t* e) {
    static bool is_measuring = false;
    
    if (!is_measuring) {
        if (!sdManager.is_initialized() && !sdManager.init(Config::SD_CS)) {
            handle_error("SD 카드 초기화 실패");
            return;
        }
        
        if (!sdManager.open_heart_data_file()) {
            handle_error("심박 데이터 파일 생성 실패");
            return;
        }
        
        is_measuring = true;
        lv_timer_create(measure_heart_rate, HEART_MEASURE_INTERVAL, NULL);
        lv_obj_add_state(ui_measuring, LV_STATE_CHECKED);
        
        // 측정 시작 피드백
        lv_label_set_text(ui_heartStatus, "측정 시작...");
        play_touch_sound();
    } else {
        is_measuring = false;
        sdManager.close_heart_data_file();
        lv_obj_clear_state(ui_measuring, LV_STATE_CHECKED);
        
        // 측정 종료 피드백
        lv_label_set_text(ui_heartStatus, "측정 완료");
        play_touch_sound();
    }
}

void UIEventManager::handle_relay_command(const char command, const char* device_name) {
    const int MAX_RETRIES = 3;
    int retry_count = 0;
    bool success = false;
    
    while (retry_count < MAX_RETRIES && !success) {
        Serial.printf("%s 제어 명령 전송 시도 %d/%d...\n", 
                     device_name, retry_count + 1, MAX_RETRIES);
                     
        Wire.beginTransmission(SLAVE_ADDR);
        Wire.write(command);
        int error = Wire.endTransmission();
        
        if (error == 0) {
            success = true;
            Serial.printf("%s 제어 명령 전송 성공\n", device_name);
        } else {
            retry_count++;
            if (retry_count < MAX_RETRIES) {
                delay(100);  // 재시도 전 대기
            }
        }
    }
    
    if (!success) {
        char error_msg[64];
        snprintf(error_msg, sizeof(error_msg), 
                "%s 제어 명령 전송 실패", device_name);
        handle_error(error_msg);
        return;
    }
    
    play_touch_sound();
    lv_timer_create(update_relay_status, 300, NULL);
}

void UIEventManager::handle_fan_toggle(lv_event_t* e) {
    handle_relay_command('T', "팬");
}

void UIEventManager::handle_led_toggle(lv_event_t* e) {
    handle_relay_command('L', "LED");
}

void UIEventManager::handle_smog_toggle(lv_event_t* e) {
    handle_relay_command('H', "가습기");
}

void UIEventManager::update_sensor_readings(lv_timer_t* timer) {
    if (!lv_obj_is_valid(ui_home) || !lv_obj_is_valid(ui_control)) {
        return;  // UI 객체가 유효하지 않으면 즉시 리턴
    }
    static unsigned long last_update = 0;
    static unsigned long last_reconnect_try = 0;
    static const unsigned long UPDATE_INTERVAL = 1000;  // 1초
    static const unsigned long RECONNECT_INTERVAL = 5000;  // 5초
    
    unsigned long current_time = millis();
    
    // 업데이트 간격 체크
    if (current_time - last_update < UPDATE_INTERVAL) {
        return;
    }
    
    // 시스템 상태 체크
    SystemMonitor::update();
    
    // 센서 연결 체크 및 재연결 시도
    if (!check_sensor_connection()) {
        if (current_time - last_reconnect_try >= RECONNECT_INTERVAL) {
            Serial.println("센서 재연결 시도...");
            if (sensorManager.init()) {
                Serial.println("센서 재연결 성공");
            } else {
                handle_error("센서 재연결 실패");
            }
            last_reconnect_try = current_time;
        }
        return;
    }

    // 센서 데이터 읽기
    SensorReadings readings = sensorManager.get_readings();
    if (!readings.is_valid) {
        log_error("유효하지 않은 센서 데이터");
        return;
    }

    // UI 업데이트 (버퍼 오버플로우 방지)
    lv_obj_t* screens[] = {ui_home, ui_control, ui_heartmeasuring};
    for (auto screen : screens) {
        if (lv_scr_act() == screen) {
            update_temperature_display(readings.temperature);
            update_humidity_display(readings.humidity);
            
            if (readings.ir_value > 50000) {
                update_heart_rate_display(readings.heart_rate, readings.spo2);
            } else {
                if (lv_obj_is_valid(ui_heartData)) {
                    lv_label_set_text(ui_heartData, "손가락을\n올려주세요");
                }
            }
            break;
        }
    }
    
    last_update = current_time;
}

void UIEventManager::update_relay_status(lv_timer_t* timer) {
    Wire.requestFrom(SLAVE_ADDR, 3);  // 팬, LED, 가습기 상태 요청
    
    if (Wire.available() >= 3) {
        update_relay_display("Fan", Wire.read() == '1');
        update_relay_display("LED", Wire.read() == '1');
        update_relay_display("Smog", Wire.read() == '1');
    } else {
        handle_error("릴레이 상태 읽기 실패");
    }
}

void UIEventManager::update_temperature_display(float temp) {
    if (!lv_obj_is_valid(ui_tempData) || !lv_obj_is_valid(ui_tempBar)) {
        return;
    }
    
    static char temp_text[32];
    snprintf(temp_text, sizeof(temp_text), "%.1f °C", temp);
    
    lv_label_set_text(ui_tempData, temp_text);
    lv_bar_set_value(ui_tempBar, (int)temp, LV_ANIM_ON);
}

void UIEventManager::update_humidity_display(float hum) {
    if (!lv_obj_is_valid(ui_humData) || !lv_obj_is_valid(ui_humBar)) {
        return;
    }
    
    static char hum_text[32];
    snprintf(hum_text, sizeof(hum_text), "%.1f %%", hum);
    
    lv_label_set_text(ui_humData, hum_text);
    lv_bar_set_value(ui_humBar, (int)hum, LV_ANIM_ON);
}

void UIEventManager::update_heart_rate_display(float hr, float spo2) {
    if (!lv_obj_is_valid(ui_heartData)) {
        return;
    }
    
    static char heart_text[64];
    snprintf(heart_text, sizeof(heart_text), 
             "심박수: %.0f bpm\n산소포화도: %.0f%%", hr, spo2);
    
    lv_label_set_text(ui_heartData, heart_text);
}

void UIEventManager::update_relay_display(const char* device, bool state) {
    if (strcmp(device, "Fan") == 0) {
        lv_label_set_text(ui_fan, state ? "Fan ON" : "Fan OFF");
    } else if (strcmp(device, "LED") == 0) {
        lv_label_set_text(ui_led, state ? "LED ON" : "LED OFF");
    } else if (strcmp(device, "Smog") == 0) {
        lv_label_set_text(ui_humidifier, state ? "Smog ON" : "Smog OFF");
    }
}

void UIEventManager::play_touch_sound() {
    if (!sdManager.is_initialized()) {
        if (!sdManager.init(Config::SD_CS)) {
            handle_error("SD 카드 초기화 실패");
            return;
        }
    }
    
    sdManager.play_touch_sound();
}

void UIEventManager::handle_error(const char* error_msg) {
    Serial.println(error_msg);
    // TODO: 에러 로깅 구현
}

bool UIEventManager::check_sensor_connection() {
    return sensorManager.check_connection();
}

void UIEventManager::log_error(const char* error_msg) {
    // 에러 로깅
    if (State::isSdCardInitialized) {
        File logFile = SD.open("/error.log", FILE_APPEND);
        if (logFile) {
            char timestamp[32];
            snprintf(timestamp, sizeof(timestamp), "[%lu] ", millis());
            logFile.print(timestamp);
            logFile.println(error_msg);
            logFile.close();
        }
    }
    
    // 시리얼 출력
    Serial.println(error_msg);
}





