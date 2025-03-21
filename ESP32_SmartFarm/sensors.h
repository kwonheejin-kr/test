#ifndef SENSORS_H
#define SENSORS_H

#include <cstdint>
#include <Arduino.h>

#ifdef __cplusplus
extern "C" {
#endif

// C++ 클래스 정의 전에 필요한 구조체 정의
struct SensorReadings {
    float temperature;
    float humidity;
    float heart_rate;
    float spo2;
    float distance;
    uint32_t ir_value;
    uint32_t timestamp;
    bool is_valid;
};

// C 인터페이스 함수들
bool sensors_init(void);
SensorReadings get_sensor_readings(void);
bool check_sensors_status(void);
float read_temperature(void);
float read_humidity(void);
float read_heart_rate(void);
float read_spo2(void);
float read_distance(void);
uint32_t get_ir_value(void);

#ifdef __cplusplus
}

// C++ 클래스 정의
class SensorManager {
public:
    SensorManager();
    bool init();
    bool check_sensors_status();
    SensorReadings get_readings();
    bool save_heart_data(float bpm, float spo2);
    
    // 누락된 함수 선언들 추가
    bool validate_readings(const SensorReadings& readings);
    float read_temperature();
    float read_humidity();
    float read_heart_rate();
    float read_spo2();
    float read_distance();
    uint32_t get_ir_value();

private:
    bool sensors_initialized;
    SensorReadings last_readings;
};

// 전역 객체 선언
extern SensorManager sensorManager;

#endif  // __cplusplus
#endif  // SENSORS_H
