#ifndef CONFIG_H
#define CONFIG_H

struct Config {
    static const uint8_t I2C_SLAVE_ADDR = 0x08;
    static const uint8_t SD_CS = 5;
    static const uint8_t SHT31_ADDR = 0x44;
    static const unsigned long WIFI_TIMEOUT = 30000;
    
    // I2S 설정
    static const uint8_t I2S_BCLK = 26;    // I2S BCLK
    static const uint8_t I2S_LRC = 25;     // I2S WCLK
    static const uint8_t I2S_DOUT = 22;    // I2S DOUT
    
    // Bluetooth Audio 설정
    static constexpr const char* BT_DEVICE_NAME = "ESP32_SmartFarm_Audio";
};

#endif

