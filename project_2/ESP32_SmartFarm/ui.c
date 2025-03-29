// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.5.0
// LVGL version: 9.1.0
// Project name: SquareLine_Project_change

#include "ui.h"
#include "ui_helpers.h"

///////////////////// VARIABLES ////////////////////


// SCREEN: ui_home
void ui_home_screen_init(void);
void ui_event_home(lv_event_t * e);
lv_obj_t * ui_home;
lv_obj_t * ui_backgroundImg;
void ui_event_rightHighBOT1(lv_event_t * e);
lv_obj_t * ui_rightHighBOT1;
void ui_event_leftHighBOT(lv_event_t * e);
lv_obj_t * ui_leftHighBOT;
void ui_event_bluetooth1(lv_event_t * e);
lv_obj_t * ui_bluetooth1;
lv_obj_t * ui_leftHigh;
lv_obj_t * ui_rightHigh;
lv_obj_t * ui_leftLow;
lv_obj_t * ui_Temp;
lv_obj_t * ui_Hum;
lv_obj_t * ui_tempBar;
lv_obj_t * ui_humBar;
lv_obj_t * ui_tempset;
lv_obj_t * ui_humset;
lv_obj_t * ui_humData;
lv_obj_t * ui_tempData;
// CUSTOM VARIABLES


// SCREEN: ui_control
void ui_control_screen_init(void);
lv_obj_t * ui_control;
lv_obj_t * ui_Image11;
void ui_event_ControlLeftUp(lv_event_t * e);
lv_obj_t * ui_ControlLeftUp;
void ui_event_ControlLeftDown(lv_event_t * e);
lv_obj_t * ui_ControlLeftDown;
void ui_event_ControlRightUp(lv_event_t * e);
lv_obj_t * ui_ControlRightUp;
void ui_event_ControlRightDown(lv_event_t * e);
lv_obj_t * ui_ControlRightDown;
lv_obj_t * ui_Humidifier;
lv_obj_t * ui_fan;
lv_obj_t * ui_led;
lv_obj_t * ui_ControlBack;
// CUSTOM VARIABLES


// SCREEN: ui_heartmeasuring
void ui_heartmeasuring_screen_init(void);
void ui_event_heartmeasuring(lv_event_t * e);
lv_obj_t * ui_heartmeasuring;
lv_obj_t * ui_Image12;
lv_obj_t * ui_Label16;
void ui_event_HeartLeftUp(lv_event_t * e);
lv_obj_t * ui_HeartLeftUp;
lv_obj_t * ui_Measuring;
lv_obj_t * ui_HeartBack;
// CUSTOM VARIABLES


// SCREEN: ui_bluetooth
void ui_bluetooth_screen_init(void);
void ui_event_bluetooth(lv_event_t * e);
lv_obj_t * ui_bluetooth;
lv_obj_t * ui_Image1;
lv_obj_t * ui_Label1;
void ui_event_blueLeftup(lv_event_t * e);
lv_obj_t * ui_blueLeftup;
lv_obj_t * ui_bluetoothLabel;
lv_obj_t * ui_bluetoothBack;
// CUSTOM VARIABLES

// EVENTS
lv_obj_t * ui____initial_actions0;

// IMAGES AND IMAGE SETS
const lv_image_dsc_t * ui_imgset_bluetooth[1] = {&ui_img_bluetooth2_png};

///////////////////// TEST LVGL SETTINGS ////////////////////
#if LV_COLOR_DEPTH != 16
    #error "LV_COLOR_DEPTH should be 16bit to match SquareLine Studio's settings"
#endif

///////////////////// ANIMATIONS ////////////////////

///////////////////// FUNCTIONS ////////////////////
void ui_event_home(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_SCREEN_LOADED) {
        update_Sensor_Data(e);
    }
}

void ui_event_rightHighBOT1(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_CLICKED) {
        _ui_screen_change(&ui_heartmeasuring, LV_SCR_LOAD_ANIM_FADE_ON, 250, 0, &ui_heartmeasuring_screen_init);
    }
}

void ui_event_leftHighBOT(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_CLICKED) {
        _ui_screen_change(&ui_control, LV_SCR_LOAD_ANIM_FADE_ON, 250, 0, &ui_control_screen_init);
    }
}

void ui_event_bluetooth1(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_CLICKED) {
        _ui_screen_change(&ui_bluetooth, LV_SCR_LOAD_ANIM_FADE_ON, 250, 0, &ui_bluetooth_screen_init);
    }
}

void ui_event_ControlLeftUp(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_PRESSED) {
        _ui_screen_change(&ui_home, LV_SCR_LOAD_ANIM_FADE_ON, 250, 0, &ui_home_screen_init);
    }
}

void ui_event_ControlLeftDown(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_CLICKED) {
        fan_toggle(e);
    }
}

void ui_event_ControlRightUp(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_CLICKED) {
        toggle_smog(e);
    }
}

void ui_event_ControlRightDown(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_CLICKED) {
        toggle_led(e);
    }
}

void ui_event_heartmeasuring(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_SCREEN_LOAD_START) {
        HeartBeatMeasuringStart(e);
    }
}

void ui_event_HeartLeftUp(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_PRESSED) {
        _ui_screen_change(&ui_home, LV_SCR_LOAD_ANIM_FADE_ON, 250, 0, &ui_home_screen_init);
    }
}

void ui_event_bluetooth(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_SCREEN_LOAD_START) {
        start_bluetooth_connection(e);
    }
}

void ui_event_blueLeftup(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_PRESSED) {
        _ui_screen_change(&ui_home, LV_SCR_LOAD_ANIM_FADE_ON, 250, 0, &ui_home_screen_init);
    }
}

///////////////////// SCREENS ////////////////////

void ui_init(void)
{
    LV_EVENT_GET_COMP_CHILD = lv_event_register_id();

    lv_disp_t * dispp = lv_display_get_default();
    lv_theme_t * theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED),
                                               false, LV_FONT_DEFAULT);
    lv_disp_set_theme(dispp, theme);
    ui_home_screen_init();
    ui_control_screen_init();
    ui_heartmeasuring_screen_init();
    ui_bluetooth_screen_init();
    ui____initial_actions0 = lv_obj_create(NULL);
    lv_disp_load_scr(ui_home);
}
