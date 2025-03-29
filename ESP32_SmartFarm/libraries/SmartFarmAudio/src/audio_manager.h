#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include <Arduino.h>
#include <AudioFileSourceSD.h>
#include <AudioGeneratorWAV.h>
#include <AudioOutputI2S.h>
#include <State.h>
#include <Config.h>

class AudioManager {
public:
    AudioManager();
    ~AudioManager();
    
    bool init();
    bool play_touch_sound();
    bool play_sound(const char* filename);
    void handle_audio();
    void stop_sound();
    bool is_playing() const;

private:
    static const char* TOUCH_SOUND_FILE;
    AudioOutputI2S* audio_out;
    AudioGeneratorWAV* current_wav;
    AudioFileSourceSD* current_file;
    
    bool init_i2s();
    void cleanup_audio();
    bool check_file_exists(const char* filename);
};

extern AudioManager audioManager;

#endif
