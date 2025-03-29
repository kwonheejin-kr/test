#ifndef STATE_H
#define STATE_H

class SDManager;      // 전방 선언 추가
class SensorManager;  // 전방 선언 추가

struct State {
    static bool isSdCardInitialized;
    static bool isWifiConnected;
    static bool isAudioPlaying;
};

struct Timers {
    static unsigned long lastWiFiRetry;
    static unsigned long lastSensorUpdate;
    static unsigned long lastUIUpdate;
    
    static void update(unsigned long currentMillis);
};

extern SensorManager sensorManager;
extern SDManager sdManager;

#endif

