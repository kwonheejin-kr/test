#include "ui_components.h"

// UI 화면 객체들
lv_obj_t * ui_home = NULL;
lv_obj_t * ui_control = NULL;
lv_obj_t * ui_heartmeasuring = NULL;
lv_obj_t * ui_bluetooth = NULL;

// Bluetooth 화면 객체들
lv_obj_t * ui_Image1 = NULL;
lv_obj_t * ui_Label1 = NULL;
lv_obj_t * ui_blueLeftup = NULL;
lv_obj_t * ui_bluetoothLabel = NULL;
lv_obj_t * ui_bluetoothBack = NULL;

// 이벤트 핸들러 구현
void ui_event_blueLeftup(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED) {
        // 뒤로가기 처리
        lv_disp_load_scr(ui_home);
    }
}

void ui_event_bluetooth(lv_event_t * e) {
    // Bluetooth 이벤트 처리
}

void get_component_child_event_cb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t ** c = lv_event_get_user_data(e);
    if(code == LV_EVENT_GET_COMP_CHILD) {
        uint32_t child_idx = lv_event_get_param(e);
        if(child_idx < _UI_COMP_BLUETOOTH_NUM) {
            lv_event_set_return_value(e, c[child_idx]);
        }
    }
}

void del_component_child_event_cb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t ** c = lv_event_get_user_data(e);
    if(code == LV_EVENT_DELETE) {
        lv_free(c);
    }
}







