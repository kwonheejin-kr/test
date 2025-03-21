#ifndef UI_HELPERS_H
#define UI_HELPERS_H

#include "lvgl.h"
#include "ui_events.h"  // UI 이벤트 관련 헤더 추가

// UI 헬퍼 함수
lv_obj_t * create_button(lv_obj_t * parent, const char * text, lv_event_cb_t event_cb);
void heart_event_handler(lv_event_t * e);
void control_event_handler(lv_event_t * e);

// 팬, LED, 스모그 이벤트
void toggle_fan(lv_event_t * e);
void toggle_led(lv_event_t * e);
void toggle_smog(lv_event_t * e);

#endif // UI_HELPERS_H
