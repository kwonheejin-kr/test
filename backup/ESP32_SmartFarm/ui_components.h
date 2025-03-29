#ifndef UI_COMPONENTS_H
#define UI_COMPONENTS_H

#include "lv_conf.h"
#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

// UI 객체 선언
extern lv_obj_t * ui_bluetooth;
extern lv_obj_t * ui_Image1;
extern lv_obj_t * ui_Label1;
extern lv_obj_t * ui_blueLeftup;
extern lv_obj_t * ui_bluetoothLabel;
extern lv_obj_t * ui_bluetoothBack;

// 이미지 선언
extern const lv_img_dsc_t ui_img_1086554574;
extern const lv_img_dsc_t ui_img_back_png;

// 이벤트 정의
enum {
    LV_EVENT_GET_COMP_CHILD = LV_EVENT_CUSTOM_BASE,
    LV_EVENT_SET_COMP_CHILD,
    LV_EVENT_DELETE_COMP_CHILD
};

// 이벤트 핸들러 선언
void ui_event_blueLeftup(lv_event_t * e);
void ui_event_bluetooth(lv_event_t * e);
void get_component_child_event_cb(lv_event_t * e);
void del_component_child_event_cb(lv_event_t * e);

// UI 초기화 함수
void ui_init(void);
void ui_bluetooth_screen_init(void);

#ifdef __cplusplus
}
#endif

#endif // UI_COMPONENTS_H



