#include "sensors.h"

#ifdef USE_DHT_SENSOR
DHT dht(DHT_PIN, DHT_TYPE);
#endif

#ifdef USE_VL53L0X_SENSOR
Adafruit_VL53L0X distanceSensor;
#endif

#ifdef USE_MAX30102_SENSOR
Adafruit_MAX30102 heartRateSensor;
#endif

void init_sensors() {
#ifdef USE_DHT_SENSOR
    dht.begin();
    Serial.println("DHT22 센서 초기화 완료");
#endif

#ifdef USE_VL53L0X_SENSOR
    if (!distanceSensor.begin()) {
        Serial.println("VL53L0X 초기화 실패");
    } else {
        Serial.println("VL53L0X 센서 초기화 완료");
    }
#endif

#ifdef USE_MAX30102_SENSOR
    if (!heartRateSensor.begin()) {
        Serial.println("MAX30102 초기화 실패");
    } else {
        Serial.println("MAX30102 센서 초기화 완료");
    }
#endif
}

float read_temperature() {
    #ifdef USE_DHT_SENSOR
    float temp = dht.readTemperature();
    return isnan(temp) ? -1 : temp;
    #else
    return -1;
    #endif
}

float read_humidity() {
    #ifdef USE_DHT_SENSOR
    float hum = dht.readHumidity();
    return isnan(hum) ? -1 : hum;
    #else
    return -1;
    #endif
}

float read_distance() {
    #ifdef USE_VL53L0X_SENSOR
    VL53L0X_RangingMeasurementData_t measure;
    distanceSensor.rangingTest(&measure, false);
    return (measure.RangeMilliMeter <= 0) ? -1 : measure.RangeMilliMeter;
    #else
    return -1;
    #endif
}

float read_heart_rate() {
    #ifdef USE_MAX30102_SENSOR
    float heartRate = heartRateSensor.getHeartRate();
    return (heartRate <= 0) ? -1 : heartRate;
    #else
    return -1;
    #endif
}
