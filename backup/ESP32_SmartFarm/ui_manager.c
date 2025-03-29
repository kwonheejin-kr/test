// 1. UI 기본 라이브러리
#include "lvgl.h"

// 2. UI 관련 헤더
#include "ui_manager.h"
#include "ui_components.h"
#include "ui_helpers.h"
#include "ui_events.h"

// 3. 프로젝트 컴포넌트
#include "sensors.h"
#include "actuator_control.h"
#include "comms.h"
#include "sd_utils.h"

lv_obj_t * ui_home;
lv_obj_t * ui_control;
lv_obj_t * ui_heartmeasuring;
lv_obj_t * ui_bluetooth;

// UI 초기화 함수
void initUI() {
    lv_init();
    lv_disp_t * disp = lv_display_get_default();
    lv_theme_t * theme = lv_theme_default_init(disp, lv_palette_main(LV_PALETTE_BLUE),
                                               lv_palette_main(LV_PALETTE_RED), false, LV_FONT_DEFAULT);
    lv_disp_set_theme(disp, theme);

    // 화면 생성
    create_home_screen();
    create_control_screen();
    create_heartmeasuring_screen();
    create_bluetooth_screen();
    
    lv_disp_load_scr(ui_home);
}

// UI 이벤트 루프 실행
void handleUIEvents() {
    lv_timer_handler();
}

// 화면 전환 함수
void switchScreen(lv_obj_t * screen) {
    lv_scr_load_anim(screen, LV_SCR_LOAD_ANIM_FADE_ON, 250, 0, false);
}

