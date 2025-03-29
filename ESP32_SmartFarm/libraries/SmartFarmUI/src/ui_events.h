#include <Arduino.h>
#include <lvgl.h>
#include <State.h>

#ifndef UI_EVENTS_H
#define UI_EVENTS_H

#include "lvgl.h"
#include "ui_components.h"

#ifdef __cplusplus
extern "C" {
#endif

// C 인터페이스
void ui_event_fan_toggle(lv_event_t* e);
void ui_event_led_toggle(lv_event_t* e);
void ui_event_smog_toggle(lv_event_t* e);
void ui_event_heart_measure_start(lv_event_t* e);
void ui_event_sensor_update(lv_event_t* e);
void ui_event_button_press(lv_event_t* e);

#ifdef __cplusplus
}

// C++ 클래스
class UIEventManager {
public:
    static void init();
    
    // UI 이벤트 핸들러
    static void handle_fan_toggle(lv_event_t* e);
    static void handle_led_toggle(lv_event_t* e);
    static void handle_smog_toggle(lv_event_t* e);
    static void handle_heart_measure_start(lv_event_t* e);
    static void handle_sensor_update(lv_event_t* e);
    static void handle_button_press(lv_event_t* e);
    
    // 타이머 콜백
    static void update_sensor_readings(lv_timer_t* timer);
    static void update_relay_status(lv_timer_t* timer);
    static void measure_heart_rate(lv_timer_t* timer);
    
    // 상태 업데이트 함수
    static void update_temperature_display(float temp);
    static void update_humidity_display(float hum);
    static void update_heart_rate_display(float hr, float spo2);
    static void update_relay_display(const char* device, bool state);

private:
    static const uint32_t SENSOR_UPDATE_INTERVAL = 2000;
    static const uint32_t RELAY_CHECK_INTERVAL = 300;
    static const uint32_t HEART_MEASURE_INTERVAL = 100;
    
    static void play_touch_sound();
    static void handle_error(const char* error_msg);
    static bool check_sensor_connection();
    
    UIEventManager() = delete;
};

#endif
#endif
