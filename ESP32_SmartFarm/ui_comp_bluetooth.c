#include "ui_components.h"

// 이벤트 정의를 수정
#define LV_EVENT_GET_COMP_CHILD (LV_EVENT_CUSTOM_BASE + 1)

void ui_event_comp_bluetooth_bluetooth(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t ** comp_bluetooth = lv_event_get_user_data(e);

    if(event_code == LV_EVENT_CLICKED) {
        hide_home_button(e);
    }
}

lv_obj_t * ui_bluetooth_create(lv_obj_t * comp_parent)
{
    lv_obj_t * cui_bluetooth;
    cui_bluetooth = lv_imagebutton_create(comp_parent);
    lv_imagebutton_set_src(cui_bluetooth, LV_IMAGEBUTTON_STATE_RELEASED, NULL, &ui_img_bluetooth2_png, NULL);
    lv_obj_set_width(cui_bluetooth, 64);
    lv_obj_set_height(cui_bluetooth, 57);
    lv_obj_set_x(cui_bluetooth, -101);
    lv_obj_set_y(cui_bluetooth, 45);
    lv_obj_set_align(cui_bluetooth, LV_ALIGN_CENTER);

    lv_obj_t ** children = lv_malloc(sizeof(lv_obj_t *) * _UI_COMP_BLUETOOTH_NUM);
    children[UI_COMP_BLUETOOTH_BLUETOOTH] = cui_bluetooth;
    
    // 이벤트 콜백 등록
    lv_obj_add_event_cb(cui_bluetooth, get_component_child_event_cb, LV_EVENT_GET_COMP_CHILD, children);
    lv_obj_add_event_cb(cui_bluetooth, del_component_child_event_cb, LV_EVENT_DELETE, children);
    lv_obj_add_event_cb(cui_bluetooth, ui_event_comp_bluetooth_bluetooth, LV_EVENT_ALL, children);
    
    ui_comp_bluetooth_create_hook(cui_bluetooth);
    return cui_bluetooth;
}

