#ifndef UI_COMPONENTS_H
#define UI_COMPONENTS_H

#include "lvgl.h"
#include "ui_helpers.h"

#ifdef __cplusplus
extern "C" {
#endif

// UI 화면 생성 함수
void create_home_screen();
void create_control_screen();
void create_heartmeasuring_screen();
void create_bluetooth_screen();

extern lv_obj_t * ui_home;
extern lv_obj_t * ui_control;
extern lv_obj_t * ui_heartmeasuring;
extern lv_obj_t * ui_bluetooth;
extern lv_obj_t * ui_Measuring;

#ifdef __cplusplus
}
#endif

#endif // UI_COMPONENTS_H
