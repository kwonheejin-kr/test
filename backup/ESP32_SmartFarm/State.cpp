#include "State.h"
#include "sensors.h"
#include "ui_events.h"
#include "lvgl.h"

bool State::isSdCardInitialized = false;
bool State::isWifiConnected = false;
bool State::isAudioPlaying = false;

unsigned long Timers::lastWiFiRetry = 0;
unsigned long Timers::lastSensorUpdate = 0;
unsigned long Timers::lastUIUpdate = 0;

void Timers::update(unsigned long currentMillis) {
    if (currentMillis - lastSensorUpdate >= 2000) {
        if (sensorManager.check_sensors_status()) {
            SensorReadings readings = sensorManager.get_readings();
            if (readings.is_valid) {
                UIEventManager::update_sensor_readings(nullptr);
            }
        }
        lastSensorUpdate = currentMillis;
    }
    
    if (currentMillis - lastUIUpdate >= 50) {
        lv_timer_handler();
        lastUIUpdate = currentMillis;
    }
}