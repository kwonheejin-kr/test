#ifndef SD_UTILS_H
#define SD_UTILS_H

#include <Arduino.h>
#include <SD.h>
#include "State.h"
#include "Config.h"
#include "AudioFileSourceSD.h"
#include "AudioGeneratorWAV.h"
#include "AudioOutputI2S.h"

class SDManager {
public:
    SDManager();
    ~SDManager();
    
    bool init(uint8_t cs_pin);
    bool play_touch_sound();
    bool play_sound(const char* filename);
    bool save_heart_data(float bpm, float spo2);
    void handle_audio();
    void close();
    bool is_initialized() const { return initialized; }

private:
    static const char* TOUCH_SOUND_FILE;
    static const size_t BUFFER_SIZE = 512;
    
    bool initialized;
    uint8_t cs_pin;
    AudioOutputI2S* audio_out;
    AudioGeneratorWAV* current_wav;
    AudioFileSourceSD* current_file;
    char buffer[BUFFER_SIZE];
    
    bool init_i2s();
    void cleanup_audio();
};

extern SDManager sdManager;

#endif
