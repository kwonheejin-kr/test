#include "sensors.h"

#ifdef USE_SHT31_SENSOR
SHT31 sht31(SHT31_PIN, SHT31_TYPE);
#endif

#ifdef USE_VL53L0X_SENSOR
Adafruit_VL53L0X distanceSensor;
#endif

#ifdef USE_MAX30102_SENSOR
MAX30105 heartRateSensor;
#endif

void init_sensors() {
#ifdef USE_SHT31_SENSOR
    sht31.begin();
    Serial.println("SHT31 센서 초기화 완료");
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

uint32_t get_ir_value() {
    return heartRateSensor.getIR();
}

float get_heart_rate() {
  uint32_t irValue = get_ir_value();

  if (checkForBeat(irValue)){
    return 60.0 * 1000.0 / (millis() - start_time);  // 심박수 계산
    }
    return -1;
}

float get_spo2() {
  uint32_t irBuffer[100], redBuffer[100];
  for (int i = 0; i < 100; i++) {
    irBuffer[i] = get_ir_value();
    redBuffer[i] = get_ir_value();
  }

  float spo2;
  int validSpO2;

  maxim_heart_rate_and_oxygen_saturation(irBuffer, redBuffer, 100, &spo2, &validSpO2);

  return validSpO2 ? spo2 : -1;
}


float read_temperature() {
    #ifdef USE_SHT31_SENSOR
    float temp = sht31.readTemperature();
    return isnan(temp) ? -1 : temp;
    #else
    return -1;
    #endif
}

float read_humidity() {
    #ifdef USE_SHT31_SENSOR
    float hum = sht31.readHumidity();
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
    return get_heart_rate();
    #else
    return -1;
    #endif
}
