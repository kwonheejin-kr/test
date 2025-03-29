#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SHT31.h>
#include <MAX30105.h>
#include <heartRate.h>
#include <spo2_algorithm.h>
#include "../../include/Config.h"

struct SensorReadings {
    float temperature;
    float humidity;
    float heart_rate;
    float spo2;
    uint16_t ir_value;
    float distance;
    unsigned long timestamp;
    bool is_valid;
};

class SensorManager {
public:
    bool init();
    bool check_sensors_status();
    SensorReadings get_readings();
    bool check_connection();
    void update();

private:
    SparkFun_MAX3010X heartRateSensor;
    Adafruit_SHT31 sht31Sensor;
    
    SensorReadings last_readings;
    bool sensors_initialized = false;
    
    float read_temperature();
    float read_humidity();
    float read_heart_rate();
    float read_spo2();
    float read_distance();
    uint16_t get_ir_value();
    bool validate_readings(const SensorReadings& readings);
};

extern SensorManager sensorManager;

#endif