#ifndef ACTUATOR_CONTROL_H
#define ACTUATOR_CONTROL_H

#include <Arduino.h>
#include <Config.h>

// 릴레이 제어 함수
void toggle_fan();
void toggle_led();
void toggle_smog();

// 릴레이 상태 확인 함수
bool get_fan_status();
bool get_led_status();
bool get_smog_status();

// 릴레이 상태 업데이트 함수
void update_relay_state(const char* device, bool state);

#endif
