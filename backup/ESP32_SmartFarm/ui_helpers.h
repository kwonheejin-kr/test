#ifndef UI_HELPERS_H
#define UI_HELPERS_H

#include "lvgl.h"
#include "ui_events.h"

// UI helper functions
lv_obj_t * create_button(lv_obj_t * parent, const char * text, lv_event_cb_t event_cb);
void heart_event_handler(lv_event_t * e);
void control_event_handler(lv_event_t * e);

// Device control events
void fan_toggle(lv_event_t * e);
void toggle_led(lv_event_t * e);
void toggle_smog(lv_event_t * e);

#endif // UI_HELPERS_H