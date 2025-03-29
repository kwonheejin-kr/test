#ifndef UI_EVENTS_H
#define UI_EVENTS_H

#include "lvgl.h"
#include "../sensors/sensors.h"

class UIEventManager {
public:
    static void init();
    static void update_sensor_readings(lv_timer_t* timer);
    static void update_relay_status(lv_timer_t* timer);
    static void update_temperature_display(float temp);
    static void update_humidity_display(float humidity);
    static void update_heart_rate_display(float hr, float spo2);
    static void update_relay_display(const char* device, bool state);
    static void play_touch_sound();
    
private:
    static bool check_sensor_connection();
    static void log_error(const char* error_msg);
    static unsigned long last_update;
    static unsigned long last_reconnect_try;
};

#endif