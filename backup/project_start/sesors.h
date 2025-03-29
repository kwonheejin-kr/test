#ifndef SENSORS_H
#define SENSORS_H

#include <DHT.h>
#include <Adafruit_VL53L0X.h>
#include <Adafruit_MAX30102.h>

#define DHT_PIN 4
#define DHT_TYPE DHT22  

// 센서 객체 선언
extern DHT dht;
extern Adafruit_VL53L0X distanceSensor;
extern Adafruit_MAX30102 heartRateSensor;

// 센서 초기화 함수
void init_sensors();

// 센서 데이터 읽기 함수
float read_temperature();
float read_humidity();
float read_distance();
float read_heart_rate();

#endif
