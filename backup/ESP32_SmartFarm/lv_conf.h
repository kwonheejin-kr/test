// LVGL 설정
#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

// 기본 설정
#define LV_USE_PERF_MONITOR     1
#define LV_COLOR_DEPTH          16
#define LV_DPI_DEF             130

// 이벤트 관련 설정
#define LV_USE_USER_DATA        1
#define LV_USE_EVENT           1
#define LV_EVENT_CUSTOM_BASE   (LV_EVENT_LAST + 1)

// 메모리 관련 설정
#define LV_MEM_CUSTOM          0
#define LV_MEM_SIZE           (32U * 1024U)

// 기타 필요한 기능 활성화
#define LV_USE_LABEL           1
#define LV_USE_IMG             1
#define LV_USE_BTN             1
#define LV_USE_IMGBTN          1

#endif // LV_CONF_H


