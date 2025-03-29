#ifndef CONFIG_H
#define CONFIG_H

struct Config {
    // 핀 설정
    static const uint8_t SD_CS = 13;
    static const uint8_t I2C_SLAVE_ADDR = 0x08;
    static const uint8_t SHT31_ADDR = 0x44;
    
    // I2S 설정
    static const uint8_t I2S_BCLK = 26;
    static const uint8_t I2S_LRC = 25;
    static const uint8_t I2S_DOUT = 22;
    
    // 타이머 설정
    static const unsigned long WIFI_TIMEOUT = 30000;
    static const unsigned long SENSOR_UPDATE_INTERVAL = 2000;
    static const unsigned long UI_UPDATE_INTERVAL = 50;
    static const unsigned long RECONNECT_INTERVAL = 5000;
    
    // Bluetooth 설정
    static constexpr const char* BT_DEVICE_NAME = "ESP32_SmartFarm_Audio";
};

#endif