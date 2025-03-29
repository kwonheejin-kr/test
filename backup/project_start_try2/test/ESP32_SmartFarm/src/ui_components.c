#include "ui_components.h"
#include "lvgl.h"
#include "ui_helpers.h"
#include "sensors.h"
#include "actuator_control.h"

// UI 객체 정의 (ui_components.h에서 extern으로 선언됨)
lv_obj_t * ui_home;
lv_obj_t * ui_control;
lv_obj_t * ui_heartmeasuring;
lv_obj_t * ui_bluetooth;
lv_obj_t * ui_Measuring;  // 🔹 `ui_Measuring` 정의 추가


void create_home_screen() {
    ui_home = lv_obj_create(NULL);
    lv_obj_remove_flag(ui_home, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * heartButton = create_button(ui_home, "Heart", heart_event_handler);
    lv_obj_align(heartButton, LV_ALIGN_CENTER, 0, -50);

    lv_obj_t * controlButton = create_button(ui_home, "Control", control_event_handler);
    lv_obj_align(controlButton, LV_ALIGN_CENTER, 0, 50);
}

void create_control_screen() {
    ui_control = lv_obj_create(NULL);
    lv_obj_remove_flag(ui_control, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * fanButton = create_button(ui_control, "Fan", toggle_fan);
    lv_obj_align(fanButton, LV_ALIGN_CENTER, 0, -60);

    lv_obj_t * ledButton = create_button(ui_control, "LED", toggle_led);
    lv_obj_align(ledButton, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t * smogButton = create_button(ui_control, "Smog", toggle_smog);
    lv_obj_align(smogButton, LV_ALIGN_CENTER, 0, 60);
}

void create_heartmeasuring_screen() {
    ui_heartmeasuring = lv_obj_create(NULL);
    lv_obj_remove_flag(ui_heartmeasuring, LV_OBJ_FLAG_SCROLLABLE);

    ui_Measuring = lv_label_create(ui_heartmeasuring);
    lv_label_set_text(ui_Measuring, "Touch to Start");
    lv_obj_align(ui_Measuring, LV_ALIGN_CENTER, 0, 0);

    lv_obj_add_event_cb(ui_Measuring, start_heart_measuring, LV_EVENT_CLICKED, NULL);
}

void create_bluetooth_screen() {
    ui_bluetooth = lv_obj_create(NULL);
    lv_obj_remove_flag(ui_bluetooth, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * bluetoothLabel = lv_label_create(ui_bluetooth);
    lv_label_set_text(bluetoothLabel, "Connecting...");
    lv_obj_align(bluetoothLabel, LV_ALIGN_CENTER, 0, 0);
}
