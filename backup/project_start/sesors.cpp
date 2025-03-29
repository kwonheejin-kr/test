#include "sensors.h"

DHT dht(DHT_PIN, DHT_TYPE);
Adafruit_VL53L0X distanceSensor;
Adafruit_MAX30102 heartRateSensor;

void init_sensors() {
    dht.begin();
    if (!distanceSensor.begin()) {
        Serial.println("VL53L0X 초기화 실패");
    }
    if (!heartRateSensor.begin()) {
        Serial.println("MAX30102 초기화 실패");
    }
}

float read_temperature() {
    return dht.readTemperature();
}

float read_humidity() {
    return dht.readHumidity();
}

float read_distance() {
    VL53L0X_RangingMeasurementData_t measure;
    distanceSensor.rangingTest(&measure, false);
    return measure.RangeMilliMeter;
}

float read_heart_rate() {
    return heartRateSensor.getHeartRate();
}
