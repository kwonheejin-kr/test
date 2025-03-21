#ifndef SENSORS_H
#define SENSORS_H

// 필요한 센서만 활성화할 수 있도록 설정
#define USE_DHT_SENSOR
#define USE_VL53L0X_SENSOR
#define USE_MAX30102_SENSOR

#ifdef USE_DHT_SENSOR
#include <DHT.h>
#define DHT_PIN 4
#define DHT_TYPE DHT22  
#endif

#ifdef USE_VL53L0X_SENSOR
#include <Adafruit_VL53L0X.h>
#endif

#ifdef USE_MAX30102_SENSOR
#include <Adafruit_MAX30102.h>
#endif


#ifdef __cplusplus
extern "C" {
#endif

void init_sensors();  // 선언만 유지

// 센서 객체 선언
#ifdef USE_DHT_SENSOR
extern DHT dht;
#endif

#ifdef USE_VL53L0X_SENSOR
extern Adafruit_VL53L0X distanceSensor;
#endif

#ifdef USE_MAX30102_SENSOR
extern Adafruit_MAX30102 heartRateSensor;
#endif

// 센서 초기화 함수
void init_sensors();

// 센서 데이터 읽기 함수
float read_temperature();
float read_humidity();
float read_distance();
float read_heart_rate();

#ifdef __cplusplus
}
#endif

#endif // SENSORS_H
