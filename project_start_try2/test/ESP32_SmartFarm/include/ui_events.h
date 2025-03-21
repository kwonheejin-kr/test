// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.5.0
// LVGL version: 9.1.0
// Project name: SquareLine_Project_change

#ifndef _UI_EVENTS_H
#define _UI_EVENTS_H
#include "ui_helpers.h"
#include "lvgl.h"
#include "ui_manager.h"

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
void start_bluetooth_status(lv_timer_t*timer);
void hide_home_button(lv_event_t * e);
void request_relay_status(lv_timer_t * timer); //모든 릴레이 상태를 한번에 요청하는 함수
void on_button_pressed(lv_event_t*e); //버튼이 눌렸을 때 호출
void play_touch_sound(); //효과음을 재생하는 함수

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
