#include "ui_helpers.h"
#include "lvgl.h"
#include "ui_events.h"

lv_obj_t * create_button(lv_obj_t * parent, const char * text, lv_event_cb_t event_cb) {
    lv_obj_t * btn = lv_btn_create(parent);
    lv_obj_t * label = lv_label_create(btn);
    lv_label_set_text(label, text);
    lv_obj_add_event_cb(btn, event_cb, LV_EVENT_CLICKED, NULL);
    return btn;
}

void heart_event_handler(lv_event_t * e) {
    switchScreen(ui_heartmeasuring);
}

void control_event_handler(lv_event_t * e) {
    switchScreen(ui_control);
}
