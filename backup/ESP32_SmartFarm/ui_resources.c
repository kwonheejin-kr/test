#include "ui_resources.h"
#include "lv_conf.h"
#include "lvgl.h"

// 이미지 데이터 배열 선언
extern const uint8_t ui_img_1086554574_data[];
extern const uint8_t ui_img_back_png_data[];
extern const uint8_t ui_img_bluetooth2_png_data[];
extern const uint8_t ui_img_bulb_png_data[];
extern const uint8_t ui_img_control_png_data[];
extern const uint8_t ui_img_fan_png_data[];
extern const uint8_t ui_img_heart_png_data[];
extern const uint8_t ui_img_smog_png_data[];

// 이미지 디스크립터 정의
const lv_img_dsc_t ui_img_1086554574 = {
    .header.cf = LV_COLOR_FORMAT_NATIVE_WITH_ALPHA,
    .header.always_zero = 0,
    .header.reserved = 0,
    .header.w = 320,
    .header.h = 174,
    .data_size = sizeof(ui_img_1086554574_data),
    .data = ui_img_1086554574_data,
    .header.magic = LV_IMAGE_HEADER_MAGIC
};

const lv_img_dsc_t ui_img_back_png = {
    .header.cf = LV_COLOR_FORMAT_NATIVE_WITH_ALPHA,
    .header.always_zero = 0,
    .header.reserved = 0,
    .header.w = 64,
    .header.h = 54,
    .data_size = sizeof(ui_img_back_png_data),
    .data = ui_img_back_png_data,
    .header.magic = LV_IMAGE_HEADER_MAGIC
};

const lv_img_dsc_t ui_img_bluetooth2_png = {
    .header.cf = LV_COLOR_FORMAT_NATIVE_WITH_ALPHA,
    .header.always_zero = 0,
    .header.reserved = 0,
    .header.w = 64,
    .header.h = 60,
    .data_size = sizeof(ui_img_bluetooth2_png_data),
    .data = ui_img_bluetooth2_png_data,
    .header.magic = LV_IMAGE_HEADER_MAGIC
};

const lv_img_dsc_t ui_img_bulb_png = {
    .header.cf = LV_COLOR_FORMAT_NATIVE_WITH_ALPHA,
    .header.always_zero = 0,
    .header.reserved = 0,
    .header.w = 64,
    .header.h = 56,
    .data_size = sizeof(ui_img_bulb_png_data),
    .data = ui_img_bulb_png_data,
    .header.magic = LV_IMAGE_HEADER_MAGIC
};

const lv_img_dsc_t ui_img_control_png = {
    .header.cf = LV_COLOR_FORMAT_NATIVE_WITH_ALPHA,
    .header.always_zero = 0,
    .header.reserved = 0,
    .header.w = 64,
    .header.h = 57,
    .data_size = sizeof(ui_img_control_png_data),
    .data = ui_img_control_png_data,
    .header.magic = LV_IMAGE_HEADER_MAGIC
};

const lv_img_dsc_t ui_img_fan_png = {
    .header.cf = LV_COLOR_FORMAT_NATIVE_WITH_ALPHA,
    .header.always_zero = 0,
    .header.reserved = 0,
    .header.w = 64,
    .header.h = 58,
    .data_size = sizeof(ui_img_fan_png_data),
    .data = ui_img_fan_png_data,
    .header.magic = LV_IMAGE_HEADER_MAGIC
};

const lv_img_dsc_t ui_img_heart_png = {
    .header.cf = LV_COLOR_FORMAT_NATIVE_WITH_ALPHA,
    .header.always_zero = 0,
    .header.reserved = 0,
    .header.w = 64,
    .header.h = 56,
    .data_size = sizeof(ui_img_heart_png_data),
    .data = ui_img_heart_png_data,
    .header.magic = LV_IMAGE_HEADER_MAGIC
};

const lv_img_dsc_t ui_img_smog_png = {
    .header.cf = LV_COLOR_FORMAT_NATIVE_WITH_ALPHA,
    .header.always_zero = 0,
    .header.reserved = 0,
    .header.w = 64,
    .header.h = 53,
    .data_size = sizeof(ui_img_smog_png_data),
    .data = ui_img_smog_png_data,
    .header.magic = LV_IMAGE_HEADER_MAGIC
};

