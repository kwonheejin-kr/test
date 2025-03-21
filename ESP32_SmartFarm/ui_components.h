#ifndef UI_COMPONENTS_H
#define UI_COMPONENTS_H

#include "lv_conf.h"
#include "lvgl.h"
#include "ui_helpers.h"
#include "ui_resources.h"

// C 코드에서 필요한 센서 데이터 구조체만 포함
struct SensorReadings;

// UI 화면 객체들
extern lv_obj_t * ui_home;
extern lv_obj_t * ui_control;
extern lv_obj_t * ui_heartmeasuring;
extern lv_obj_t * ui_bluetooth;

// Bluetooth 화면 객체들
extern lv_obj_t * ui_Image1;
extern lv_obj_t * ui_Label1;
extern lv_obj_t * ui_blueLeftup;
extern lv_obj_t * ui_bluetoothLabel;
extern lv_obj_t * ui_bluetoothBack;

// 이벤트 핸들러
void ui_event_blueLeftup(lv_event_t * e);
void ui_event_bluetooth(lv_event_t * e);
void get_component_child_event_cb(lv_event_t * e);
void del_component_child_event_cb(lv_event_t * e);      

// UI 컴포넌트 상수
#define UI_COMP_BLUETOOTH_BLUETOOTH 0
#define _UI_COMP_BLUETOOTH_NUM 1

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif

