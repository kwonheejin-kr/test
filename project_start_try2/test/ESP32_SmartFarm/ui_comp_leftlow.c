// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.5.0
// LVGL version: 9.1.0
// Project name: SquareLine_Project_change

#include "ui_components.h"


// COMPONENT leftLow

lv_obj_t * ui_leftLow_create(lv_obj_t * comp_parent)
{

    lv_obj_t * cui_leftLow;
    cui_leftLow = lv_label_create(comp_parent);
    lv_obj_set_width(cui_leftLow, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(cui_leftLow, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(cui_leftLow, -101);
    lv_obj_set_y(cui_leftLow, 90);
    lv_obj_set_align(cui_leftLow, LV_ALIGN_CENTER);
    lv_label_set_text(cui_leftLow, "bluetooth");

    lv_obj_t ** children = lv_malloc(sizeof(lv_obj_t *) * _UI_COMP_LEFTLOW_NUM);
    children[UI_COMP_LEFTLOW_LEFTLOW] = cui_leftLow;
    lv_obj_add_event_cb(cui_leftLow, get_component_child_event_cb, LV_EVENT_GET_COMP_CHILD, children);
    lv_obj_add_event_cb(cui_leftLow, del_component_child_event_cb, LV_EVENT_DELETE, children);
    ui_comp_leftLow_create_hook(cui_leftLow);
    return cui_leftLow;
}

