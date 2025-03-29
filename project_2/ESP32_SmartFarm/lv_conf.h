/**
 * @file lv_conf.h
 * LVGL 구성 파일 for v9.2.2
 * ESP32 SmartFarm 프로젝트에 최적화
 */

#ifndef LV_CONF_H
#define LV_CONF_H

#ifndef __ASSEMBLER__
// 이 부분은 컴파일러가 처리하는 부분으로, 어셈블러에서는 무시됩니다.
#include <stdint.h>
#endif

/* 색상 깊이 설정 - 8비트로 감소하여 ESP32-WROOM-32의 메모리 사용량을 줄임 */
#define LV_COLOR_DEPTH     8

/* 라이브러리가 지원하는 최대 해상도 - 실제 사용 해상도에 맞게 조정 */
#define LV_HOR_RES_MAX          (320)
#define LV_VER_RES_MAX          (240)

/* 더블 버퍼링 비활성화로 메모리 절약 */
#define LV_DISP_DEF_REFR_PERIOD   30      /*[ms]*/

/* LVGL 메모리 파일시스템 설정 */
#define LV_USE_FS_MEMFS       1
#define LV_FS_MEMFS_LETTER    'M'  /* 'M'은 Memory FileSystem의 시스템 드라이버 문자를 지정합니다 */

/* 기본 글꼴 설정 - 필요한 것만 활성화 */
#define LV_FONT_MONTSERRAT_12    1
#define LV_FONT_MONTSERRAT_14    1  /* UI 디자인에 맞게 Montserrat 14 활성화 */
#define LV_FONT_MONTSERRAT_16    1
#define LV_FONT_MONTSERRAT_18    0
#define LV_FONT_MONTSERRAT_20    0
#define LV_FONT_MONTSERRAT_22    0
#define LV_FONT_MONTSERRAT_24    0
#define LV_FONT_MONTSERRAT_26    0
#define LV_FONT_MONTSERRAT_28    0
#define LV_FONT_MONTSERRAT_30    0
#define LV_FONT_MONTSERRAT_32    0
#define LV_FONT_MONTSERRAT_34    0
#define LV_FONT_MONTSERRAT_36    0
#define LV_FONT_MONTSERRAT_38    0
#define LV_FONT_MONTSERRAT_40    0
#define LV_FONT_MONTSERRAT_42    0
#define LV_FONT_MONTSERRAT_44    0
#define LV_FONT_MONTSERRAT_48    0

/* 글꼴 압축 사용 */
#define LV_USE_FONT_COMPRESSED    1

/* 메모리 사용 설정 */
#define LV_MEM_CUSTOM      0
#define LV_MEM_SIZE        (32U * 1024U)          /*[bytes]*/

/* 로그 사용 설정 */
#define LV_USE_LOG      0

/* 기본 테마 사용 설정 */
#define LV_USE_THEME_DEFAULT 1
#define LV_THEME_DEFAULT_DARK 0

/* 애니메이션 사용 설정 */
#define LV_USE_ANIMATION 1
#define LV_USE_SHADOW    0
#define LV_USE_BLEND_MODES 0
#define LV_USE_OPACITY 0

/* 입력 장치 사용 설정 */
#define LV_USE_INDEV_TOUCHPAD  1

/* 파일 시스템 사용 설정 */
#define LV_USE_FS_FATFS        0
#define LV_USE_FS_STDIO        0
#define LV_USE_FS_POSIX        0
#define LV_USE_FS_WIN32        0
#define LV_USE_FS_MEMFS        1

/* 이미지 사용 설정 */
#define LV_USE_PNG 0
#define LV_USE_BMP 0
#define LV_USE_JPG 0
#define LV_USE_GIF 0
#define LV_USE_QRCODE 0

/* 위젯 사용 설정 */
#define LV_USE_ARC        1
#define LV_USE_BAR        1
#define LV_USE_BTN        1
#define LV_USE_BTNMATRIX  0
#define LV_USE_CANVAS     0
#define LV_USE_CHECKBOX   0
#define LV_USE_DROPDOWN   1  /* 드롭다운 위젯 사용 */
#define LV_USE_IMG        1
#define LV_USE_LABEL      1
#define LV_USE_LINE       0
#define LV_USE_ROLLER     0
#define LV_USE_SLIDER     0
#define LV_USE_SWITCH     0
#define LV_USE_TABLE      0
#define LV_USE_TEXTAREA   1  /* 텍스트 에어리어 위젯 사용 */

#endif /*LV_CONF_H*/