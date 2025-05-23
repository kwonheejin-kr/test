// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.5.0
// LVGL version: 9.1.0
// Project name: SquareLine_Project_change

#include "ui.h"

void ui_heartmeasuring_screen_init(void)
{
    ui_heartmeasuring = lv_obj_create(NULL);
    lv_obj_remove_flag(ui_heartmeasuring, LV_OBJ_FLAG_SCROLLABLE);      /// Flags

    ui_Image12 = lv_image_create(ui_heartmeasuring);
    lv_image_set_src(ui_Image12, &ui_img_1086554574);
    lv_obj_set_width(ui_Image12, LV_SIZE_CONTENT);   /// 320
    lv_obj_set_height(ui_Image12, LV_SIZE_CONTENT);    /// 174
    lv_obj_set_align(ui_Image12, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_Image12, LV_OBJ_FLAG_CLICKABLE);     /// Flags
    lv_obj_remove_flag(ui_Image12, LV_OBJ_FLAG_SCROLLABLE);      /// Flags

    ui_HeartLeftUp = lv_imagebutton_create(ui_heartmeasuring);
    lv_imagebutton_set_src(ui_HeartLeftUp, LV_IMAGEBUTTON_STATE_RELEASED, NULL, &ui_img_back_png, NULL);
    lv_obj_set_width(ui_HeartLeftUp, 64);
    lv_obj_set_height(ui_HeartLeftUp, 53);
    lv_obj_set_x(ui_HeartLeftUp, -102);
    lv_obj_set_y(ui_HeartLeftUp, -75);
    lv_obj_set_align(ui_HeartLeftUp, LV_ALIGN_CENTER);

    ui_Measuring = lv_label_create(ui_heartmeasuring);
    lv_obj_set_width(ui_Measuring, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_Measuring, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_Measuring, 0);
    lv_obj_set_y(ui_Measuring, -30);
    lv_obj_set_align(ui_Measuring, LV_ALIGN_CENTER);
    lv_label_set_text(ui_Measuring, "터치하여 측정 시작");

    ui_HeartResult = lv_label_create(ui_heartmeasuring);
    lv_obj_set_width(ui_HeartResult, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_HeartResult, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_HeartResult, 0);
    lv_obj_set_y(ui_HeartResult, 30);
    lv_obj_set_align(ui_HeartResult, LV_ALIGN_CENTER);
    lv_label_set_text(ui_HeartResult, "");

    ui_HeartBack = lv_label_create(ui_heartmeasuring);
    lv_obj_set_width(ui_HeartBack, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_HeartBack, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_HeartBack, -101);
    lv_obj_set_y(ui_HeartBack, -30);
    lv_obj_set_align(ui_HeartBack, LV_ALIGN_CENTER);
    lv_label_set_text(ui_HeartBack, "뒤로");

    lv_obj_add_event_cb(ui_HeartLeftUp, ui_event_HeartLeftUp, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(ui_heartmeasuring, ui_event_heartmeasuring, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(ui_Image12, HeartBeatMeasuringStart, LV_EVENT_CLICKED, NULL);
}
