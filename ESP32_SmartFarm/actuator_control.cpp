// 1. 통신 라이브러리
#include <Wire.h>

// 2. 프로젝트 헤더
#include "actuator_control.h"
#include "error_handler.h"

// I2C 슬레이브 주소 (Arduino R4)
const uint8_t SLAVE_ADDR = 0x08;

void toggle_fan() {
    Wire.beginTransmission(SLAVE_ADDR);
    Wire.write('T');
    Wire.endTransmission();
}

void toggle_led() {
    Wire.beginTransmission(SLAVE_ADDR);
    Wire.write('L');
    Wire.endTransmission();
}

void toggle_smog() {
    Wire.beginTransmission(SLAVE_ADDR);
    Wire.write('H');
    Wire.endTransmission();
}

bool get_fan_status() {
    Wire.requestFrom(SLAVE_ADDR, 1);
    if (Wire.available()) {
        return Wire.read() == '1';
    }
    return false;
}

bool get_led_status() {
    Wire.requestFrom(SLAVE_ADDR, 1);
    if (Wire.available()) {
        return Wire.read() == '1';
    }
    return false;
}

bool get_smog_status() {
    Wire.requestFrom(SLAVE_ADDR, 1);
    if (Wire.available()) {
        return Wire.read() == '1';
    }
    return false;
}

void update_relay_state(const char* device, bool state) {
    // UI 업데이트 로직 구현
    // 이 부분은 UI 컴포넌트와 연동되어야 합니다
}
