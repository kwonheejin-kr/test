#include "ui_components.h"
#include "ui_comp_bluetooth.h"

lv_obj_t * ui_bluetooth_create(lv_obj_t * comp_parent)
{
    lv_obj_t * cui_bluetooth = lv_obj_create(comp_parent);

    lv_obj_t ** children = lv_malloc(sizeof(lv_obj_t *) * _UI_COMP_BLUETOOTH_NUM);
    children[UI_COMP_BLUETOOTH_BLUETOOTH] = cui_bluetooth;
    
    // 이벤트 콜백 등록
    lv_obj_add_event_cb(cui_bluetooth, get_component_child_event_cb, LV_EVENT_GET_COMP_CHILD, children);
    lv_obj_add_event_cb(cui_bluetooth, del_component_child_event_cb, LV_EVENT_DELETE, children);
    lv_obj_add_event_cb(cui_bluetooth, ui_event_comp_bluetooth_bluetooth, LV_EVENT_ALL, children);
    
    ui_comp_bluetooth_create_hook(cui_bluetooth);
    return cui_bluetooth;
}






