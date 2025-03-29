// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.5.0
// LVGL version: 9.1.0
// Project name: SquareLine_Project_change
// Note: Originally generated as ui_events.c, now converted to ui_events.cpp for C++ compatibility

#ifndef _UI_EVENTS_H
#define _UI_EVENTS_H

#include "lvgl.h"

// ui_HeartResult ubcc0uc218 uc120uc5b8 ucd94uac00
extern lv_obj_t * ui_HeartResult;

#ifdef __cplusplus
extern "C" {
#endif

void update_Sensor_Data(lv_event_t * e);
void update_temp_hum(lv_timer_t * timer);
void fan_toggle(lv_event_t * e);
void request_fan_status(lv_timer_t * timer);
void toggle_smog(lv_event_t * e);
void request_smog_status(lv_timer_t * timer);  // 스모그 상태 업데이트 추가
void toggle_led(lv_event_t * e);
void request_led_status(lv_timer_t * timer);
void HeartBeatMeasuringStart(lv_event_t * e);
void measure_heart_rate(lv_timer_t * timer);
void check_sensor_data(lv_timer_t * timer);
void send_data_to_raspberry();
void start_bluetooth_connection(lv_event_t * e);
void start_bluetooth_status(lv_timer_t * timer);  // 타이머 파라미터 추가
void hide_home_button(lv_event_t * e);
void request_relay_status(lv_timer_t * timer);
void on_button_pressed(lv_event_t*e);
void a2dp_sink_init();
void play_touch_sound();
void save_measurement_data(int32_t hr, int32_t spo2);
bool check_sd_card_status();  // 반환 타입 bool로 변경
void init_data_storage();

// 오류 처리 및 최적화 함수 추가
void create_measurement_directory();
void handle_sensor_error(const char* sensor_name);
void retry_bluetooth_connection();
void power_save_mode(bool enable);

// 
void ui_event_ControlLeftDown(lv_event_t * e);
void ui_event_ControlRightDown(lv_event_t * e);
void ui_event_ControlRightUp(lv_event_t * e);
void ui_event_ControlLeftUp(lv_event_t * e);

// 
void request_fan_status(lv_timer_t * timer);
void request_led_status(lv_timer_t * timer);
void request_hum_status(lv_timer_t * timer);

// I2C 
void sendCommand(char command);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
