#include <sensors.h>
#include <Wire.h>

// 3. 센서 라이브러리들
#include <Adafruit_SHT31.h>
#include <Adafruit_VL53L0X.h>
#include <MAX30105.h>
#include <heartRate.h>
#include <spo2_algorithm.h>

Adafruit_SHT31 sht31;
Adafruit_VL53L0X distanceSensor;
MAX30105 heartRateSensor;

// 전역 변수로 심박수 및 SPO2 계산을 위한 버퍼
const uint8_t RATE_SIZE = 4; // 심박수 평균을 위한 배열 크기
uint8_t rates[RATE_SIZE]; // 심박수 배열
uint8_t rateSpot = 0;
long lastBeat = 0; // 마지막 심박 시간

// SPO2 계산을 위한 버퍼
#define BUFFER_LENGTH 100
uint32_t irBuffer[BUFFER_LENGTH];
uint32_t redBuffer[BUFFER_LENGTH];
int32_t bufferLength = 100;
int32_t spo2;
int8_t validSPO2;
int32_t heartRate;
int8_t validHeartRate;

// 전역 객체 정의
SensorManager sensorManager;

SensorManager::SensorManager() : sensors_initialized(false) {
    memset(&last_readings, 0, sizeof(last_readings));
}

bool SensorManager::init() {
    const int MAX_RETRIES = 3;
    int retry_count = 0;
    
    while (retry_count < MAX_RETRIES) {
        Serial.printf("센서 초기화 시도 %d/%d...\n", retry_count + 1, MAX_RETRIES);
        
        bool all_initialized = true;
        
        // SHT31 초기화
        if (!sht31.begin(0x44)) {
            all_initialized = false;
            handle_error("SHT31 초기화 실패");
        }

        // VL53L0X 초기화
        if (!distanceSensor.begin()) {
            all_initialized = false;
            handle_error("VL53L0X 초기화 실패");
        }

        // MAX30105 초기화 및 설정
        if (!heartRateSensor.begin()) {
            all_initialized = false;
            handle_error("MAX30105 초기화 실패");
        } else {
            // MAX30105 최적화된 설정
            heartRateSensor.setup(
                60,    // LED 밝기 (0-255)
                4,     // 샘플링 평균
                2,     // LED 모드 (RED+IR)
                411,   // 샘플링 속도
                4096,  // ADC 범위
                0      // LED 펄스 폭
            );
            
            // 추가 설정
            heartRateSensor.enableDIETEMPRDY();  // 온도 센서 활성화
            heartRateSensor.setPulseAmplitudeRed(0x0A);
            heartRateSensor.setPulseAmplitudeIR(0x0A);
        }
        
        if (all_initialized) {
            sensors_initialized = true;
            Serial.println("센서 초기화 완료");
            return true;
        }
        
        retry_count++;
        delay(1000);  // 재시도 전 대기
    }
    
    return false;
}

SensorReadings SensorManager::get_readings() {
    SensorReadings readings;
    unsigned long start_time = millis();
    
    while (!sensors_initialized) {
        if (millis() - start_time > 3000) { // 3초 타임아웃
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
        readings.distance = read_distance();
        readings.ir_value = get_ir_value();
        readings.timestamp = millis();
        
        readings.is_valid = validate_readings(readings);
        
        if (readings.is_valid) {
            last_readings = readings;
        }
    } catch (const std::exception& e) {
        handle_error("센서 읽기 중 오류 발생");
        readings = last_readings;
    }

    return readings;
}

bool SensorManager::validate_readings(const SensorReadings& readings) {
    // 온도 유효성 (-40°C ~ 125°C)
    if (readings.temperature < -40 || readings.temperature > 125) {
        Serial.println("온도 데이터 범위 초과");
        return false;
    }
    
    // 습도 유효성 (0% ~ 100%)
    if (readings.humidity < 0 || readings.humidity > 100) {
        Serial.println("습도 데이터 범위 초과");
        return false;
    }
    
    // 심박수 유효성 (30bpm ~ 250bpm)
    if (readings.ir_value > 50000 && 
        (readings.heart_rate < 30 || readings.heart_rate > 250)) {
        Serial.println("심박수 데이터 범위 초과");
        return false;
    }
    
    // SPO2 유효성 (0% ~ 100%)
    if (readings.ir_value > 50000 && 
        (readings.spo2 < 0 || readings.spo2 > 100)) {
        Serial.println("SPO2 데이터 범위 초과");
        return false;
    }

    return true;
}

float SensorManager::read_temperature() {
    if (!sensors_initialized) return -999.0f;
    float temp = sht31.readTemperature();
    return isnan(temp) ? last_readings.temperature : temp;
}

float SensorManager::read_humidity() {
    if (!sensors_initialized) return -999.0f;
    float hum = sht31.readHumidity();
    return isnan(hum) ? last_readings.humidity : hum;
}

float SensorManager::read_heart_rate() {
    if (!sensors_initialized) return -999.0f;
    
    uint32_t ir = heartRateSensor.getIR();
    if (ir < 50000) return 0;
    
    // 심박수 측정을 위한 샘플링
    bool newDataReady = false;
    for (byte i = 0; i < 10; i++) {
        if (heartRateSensor.available()) {
            ir = heartRateSensor.getIR();
            heartRateSensor.nextSample();
            newDataReady = true;
        }
    }
    
    if (!newDataReady) return last_readings.heart_rate;
    
    // 심박수 계산
    long delta = millis() - lastBeat;
    lastBeat = millis();
    
    float beatsPerMinute = 60 / (delta / 1000.0);
    
    if (beatsPerMinute < 30 || beatsPerMinute > 220) {
        return last_readings.heart_rate;
    }
    
    // 이동 평균 계산
    rates[rateSpot++] = (uint8_t)beatsPerMinute;
    rateSpot %= RATE_SIZE;
    
    float beatAvg = 0;
    for (byte x = 0; x < RATE_SIZE; x++) {
        beatAvg += rates[x];
    }
    beatAvg /= RATE_SIZE;
    
    return beatAvg;
}

float SensorManager::read_spo2() {
    if (!sensors_initialized) return -999.0f;
    
    uint32_t ir = heartRateSensor.getIR();
    uint32_t red = heartRateSensor.getRed();
    
    if (ir < 50000) return 0;
    
    // SPO2 계산을 위한 버퍼 채우기
    for (byte i = 0; i < 100; i++) {
        while (!heartRateSensor.available());
        
        irBuffer[i] = heartRateSensor.getIR();
        redBuffer[i] = heartRateSensor.getRed();
        heartRateSensor.nextSample();
    }
    
    // SPO2 계산
    maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer,
                                         &spo2, &validSPO2, &heartRate, &validHeartRate);
    
    return (validSPO2 && spo2 >= 0 && spo2 <= 100) ? spo2 : last_readings.spo2;
}

float SensorManager::read_distance() {
    if (!sensors_initialized) return -999.0f;
    
    VL53L0X_RangingMeasurementData_t measure;
    distanceSensor.rangingTest(&measure, false);
    
    if (measure.RangeStatus != 4) {
        return measure.RangeMilliMeter / 10.0f; // Convert to cm
    }
    return last_readings.distance;
}

uint32_t SensorManager::get_ir_value() {
    if (!sensors_initialized) return 0;
    return heartRateSensor.getIR();
}

bool SensorManager::check_sensors_status() {
    return sensors_initialized;
}

bool SensorManager::save_heart_data(float bpm, float spo2) {
    if (!State::isSdCardInitialized) return false;
    
    File dataFile = SD.open("/heart_data.csv", FILE_APPEND);
    if (!dataFile) {
        handle_error("심박 데이터 파일 열기 실패");
        return false;
    }
    
    // 타임스탬프 추가
    char timestamp[32];
    unsigned long current_time = millis();
    snprintf(timestamp, sizeof(timestamp), "%lu", current_time);
    
    // CSV 형식으로 데이터 저장
    dataFile.printf("%s,%.1f,%.1f\n", timestamp, bpm, spo2);
    dataFile.close();
    
    return true;
}
