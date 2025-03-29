#include "ui_events.h"
#include "../system/State.h"
#include "../system/error_handler.h"
#include "ui_components.h"

unsigned long UIEventManager::last_update = 0;
unsigned long UIEventManager::last_reconnect_try = 0;

void UIEventManager::init() {
    lv_timer_create(update_sensor_readings, SENSOR_UPDATE_INTERVAL, NULL);
    lv_timer_create(update_relay_status, RELAY_CHECK_INTERVAL, NULL);
}

void UIEventManager::update_sensor_readings(lv_timer_t* timer) {
    unsigned long current_time = millis();
    
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

    SensorReadings readings = sensorManager.get_readings();
    if (!readings.is_valid) {
        log_error("유효하지 않은 센서 데이터");
        return;
    }

    // UI 업데이트
    update_temperature_display(readings.temperature);
    update_humidity_display(readings.humidity);
    
    if (readings.ir_value > 50000) {
        update_heart_rate_display(readings.heart_rate, readings.spo2);
    } else {
        if (lv_obj_is_valid(ui_heartData)) {
            lv_label_set_text(ui_heartData, "손가락을\n올려주세요");
        }
    }
    
    last_update = current_time;
}

// 나머지 UI 이벤트 관련 메서드들 구현...