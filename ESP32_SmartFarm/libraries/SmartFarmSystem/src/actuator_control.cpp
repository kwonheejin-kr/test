#include <Wire.h>
#include "actuator_control.h"
#include "error_handler.h"

const uint8_t SLAVE_ADDR = 0x08;
const unsigned long I2C_TIMEOUT = 50; // 50ms timeout

bool send_command(char command) {
    Wire.beginTransmission(SLAVE_ADDR);
    Wire.write(command);
    byte error = Wire.endTransmission();
    
    if (error) {
        handle_error("I2C 통신 오류");
        return false;
    }
    return true;
}

bool get_device_status(char device_type) {
    unsigned long start_time = millis();
    Wire.requestFrom(SLAVE_ADDR, 1);
    
    while (!Wire.available() && millis() - start_time < I2C_TIMEOUT);
    
    if (Wire.available()) {
        return Wire.read() == '1';
    }
    
    handle_error("장치 상태 읽기 실패");
    return false;
}

void toggle_fan() {
    if (!send_command('T')) {
        handle_error("팬 토글 명령 실패");
    }
}

void toggle_led() {
    if (!send_command('L')) {
        handle_error("LED 토글 명령 실패");
    }
}

void toggle_smog() {
    if (!send_command('H')) {
        handle_error("가습기 토글 명령 실패");
    }
}

bool get_fan_status() {
    return get_device_status('T');
}

bool get_led_status() {
    return get_device_status('L');
}

bool get_smog_status() {
    return get_device_status('H');
}

void update_relay_state(const char* device, bool state) {
    // UI 업데이트 로직 구현
    // 이 부분은 UI 컴포넌트와 연동되어야 합니다
}

