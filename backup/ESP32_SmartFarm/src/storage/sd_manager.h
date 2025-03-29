#ifndef SD_MANAGER_H
#define SD_MANAGER_H

#include <Arduino.h>
#include <SD.h>
#include "../../include/Config.h"

class SDManager {
public:
    SDManager();
    ~SDManager();
    
    bool init(uint8_t cs_pin);
    bool save_heart_data(float bpm, float spo2);
    bool save_error_log(const char* error_msg);
    bool save_sensor_data(float temp, float humidity);
    void close();
    bool is_initialized() const { return initialized; }

    // 파일 관리 메서드
    bool create_directory(const char* path);
    bool file_exists(const char* path);
    size_t get_file_size(const char* path);
    bool delete_file(const char* path);

private:
    static const size_t BUFFER_SIZE = 512;
    bool initialized;
    uint8_t cs_pin;
    char buffer[BUFFER_SIZE];

    bool check_storage_space();
    void rotate_log_files();
    bool write_to_file(const char* path, const char* data, bool append = true);
};

extern SDManager sdManager;

#endif