#ifndef SENSORS_H
#define SENSORS_H

// 필요한 센서만 활성화할 수 있도록 설정
#define USE_SHT31_SENSOR
#define USE_VL53L0X_SENSOR
#define USE_MAX30102_SENSOR

#ifdef USE_SHT31_SENSOR
#include <Adafruit_SHT31.h>
#define SHT31_PIN 4
#endif

#ifdef USE_VL53L0X_SENSOR
#include <Adafruit_VL53L0X.h>
#endif

#ifdef USE_MAX30102_SENSOR
#include <MAX30105.h>
#include <heartRate.h>
#include <spo2_algorithm.h>
#endif


#ifdef __cplusplus
extern "C" {
#endif

void init_sensors();  // 선언만 유지

// 센서 객체 선언
#ifdef USE_SHT31_SENSOR
extern SHT31 sht31;
#endif

#ifdef USE_VL53L0X_SENSOR
extern Adafruit_VL53L0X distanceSensor;
#endif

#ifdef USE_MAX30102_SENSOR
extern MAX30105 heartRateSensor;
#endif

// 센서 초기화 함수
void init_sensors();
uint32_t get_ir_value(); //IR 센서 값 반환
float get_heart_rate(); // 심박수 반환
float get_spo2(); // Spo2 값 반환

// 센서 데이터 읽기 함수
float read_temperature();
float read_humidity();
float read_distance();
float read_heart_rate();

#ifdef __cplusplus
}
#endif

#endif // SENSORS_H
