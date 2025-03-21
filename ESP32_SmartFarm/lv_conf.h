#if 0
#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

/* 기본 설정 */
#define LV_USE_PERF_MONITOR     1
#define LV_USE_LOG              1
#define LV_COLOR_DEPTH          16

/* 이벤트 관련 설정 추가 */
#define LV_USE_USER_DATA        1
#define LV_USE_EVENT            1
#define LV_EVENT_CUSTOM_BASE    0x100  // 사용자 정의 이벤트 시작점

/* 화면 해상도 설정 */
#define LV_HOR_RES_MAX          320
#define LV_VER_RES_MAX          240
#define LV_DPI_DEF              130

/* 버퍼 설정 */
#define LV_DISP_DEF_REFR_PERIOD 30
#define LV_DISP_BUF_SIZE        ((LV_VER_RES_MAX * LV_HOR_RES_MAX) / 10)

/* 필수 컴포넌트 활성화 */
#define LV_USE_ANIMATION        1
#define LV_USE_SHADOW          1
#define LV_USE_GROUP           1
#define LV_USE_FILESYSTEM      1
#define LV_USE_IMG             1
#define LV_USE_LABEL           1
#define LV_USE_BUTTON          1
#define LV_USE_IMGBTN          1

/* 메모리 설정 */
#define LV_MEM_CUSTOM          0
#define LV_MEM_SIZE           (48U * 1024U)

/* 디버그 설정 */
#define LV_USE_DEBUG           1
#define LV_LOG_LEVEL          LV_LOG_LEVEL_WARN

#endif /* LV_CONF_H */
#endif


