#include "sensors.h"
#include "../system/State.h"
#include "../system/error_handler.h"

SensorManager sensorManager;

bool SensorManager::init() {
    Wire.begin();
    
    if (!heartRateSensor.begin()) {
        handle_error("MAX30102 센서 초기화 실패");
        return false;
    }
    
    heartRateSensor.setup();
    heartRateSensor.setPulseAmplitudeRed(0x0A);
    heartRateSensor.setPulseAmplitudeGreen(0);
    
    if (!sht31Sensor.begin(0x44)) {
        handle_error("SHT31 센서 초기화 실패");
        return false;
    }
    
    sensors_initialized = true;
    return true;
}

SensorReadings SensorManager::get_readings() {
    SensorReadings readings;
    unsigned long start_time = millis();
    
    if (!sensors_initialized) {
        if (millis() - start_time > 3000) {
            readings.is_valid = false;
            handle_error("센서 초기화 타임아웃");
            return readings;
        }
        delay(10);
    }

    try {
        readings.temperature = read_temperature();
        readings.humidity = read_humidity();
        readings.heart_rate = read_heart_rate();
        readings.spo2 = read_spo2();
        readings.ir_value = get_ir_value();
        readings.timestamp = millis();
        
        readings.is_valid = validate_readings(readings);
        
        if (readings.is_valid) {
            last_readings = readings;
        }
    } catch (...) {
        handle_error("센서 읽기 중 오류 발생");
        readings = last_readings;
    }

    return readings;
}

// 나머지 센서 관련 메서드들 구현...