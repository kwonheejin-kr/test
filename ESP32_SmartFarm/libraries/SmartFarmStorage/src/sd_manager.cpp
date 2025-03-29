// 1. C++ 표준 라이브러리
#include <Arduino.h>

// 2. 파일 시스템 라이브러리
#include <SD.h>

// 3. 오디오 라이브러리
#include "AudioFileSourceSD.h"
#include "AudioGeneratorWAV.h"
#include "AudioOutputI2S.h"

// 4. 프로젝트 헤더
#include "sd_manager.h"
#include "../system/error_handler.h"
#include "../system/State.h"
#include "driver/i2s.h"

const char* SDManager::TOUCH_SOUND_FILE = "/sounds/touch.wav";

// 전역 객체 정의
SDManager sdManager;

SDManager::SDManager() 
    : initialized(false)
    , cs_pin(0)
    , audio_out(nullptr)
    , current_wav(nullptr)
    , current_file(nullptr) {
}

SDManager::~SDManager() {
    close();
}

bool SDManager::init_i2s() {
    // I2S 드라이버 초기화
    static const i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = 44100,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 64,
        .use_apll = false,
        .tx_desc_auto_clear = true,
        .fixed_mclk = 0
    };
    
    if (i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL) != ESP_OK) {
        handle_error("I2S 드라이버 초기화 실패");
        return false;
    }

    static const i2s_pin_config_t pin_config = {
        .bck_io_num = Config::I2S_BCLK,    // Config에서 가져옴
        .ws_io_num = Config::I2S_LRC,      // Config에서 가져옴
        .data_out_num = Config::I2S_DOUT,  // Config에서 가져옴
        .data_in_num = I2S_PIN_NO_CHANGE
    };

    if (i2s_set_pin(I2S_NUM_0, &pin_config) != ESP_OK) {
        handle_error("I2S 핀 설정 실패");
        return false;
    }

    return true;
}

bool SDManager::init(uint8_t pin) {
    if (initialized) {
        // SD 카드가 여전히 존재하는지 더블 체크
        if (!SD.cardType()) {
            initialized = false;
            State::isSdCardInitialized = false;
        } else {
            return true;
        }
    }
    
    cs_pin = pin;
    
    if (!SD.begin(cs_pin)) {
        handle_error("SD 카드 초기화 실패");
        return false;
    }

    if (!init_i2s()) {
        SD.end();
        return false;
    }

    audio_out = new AudioOutputI2S();
    audio_out->SetPinout(Config::I2S_BCLK, Config::I2S_LRC, Config::I2S_DOUT);
    audio_out->SetGain(0.5);

    if (!SD.exists(TOUCH_SOUND_FILE)) {
        handle_error("터치음 파일이 없음");
        close();
        return false;
    }

    initialized = true;
    State::isSdCardInitialized = true;
    return true;
}

bool SDManager::save_heart_data(float bpm, float spo2) {
    if (!initialized) return false;

    File dataFile = SD.open("/heartDATA/heart_data.txt", FILE_APPEND);
    if (!dataFile) {
        handle_error("심박수 데이터 파일 열기 실패");
        return false;
    }

    size_t written = snprintf(buffer, BUFFER_SIZE, 
                            "%lu,%.1f,%.1f\n", 
                            millis(), bpm, spo2);

    if (written >= BUFFER_SIZE) {
        handle_error("버퍼 오버플로우");
        dataFile.close();
        return false;
    }

    if (dataFile.write((uint8_t*)buffer, written) != written) {
        handle_error("데이터 쓰기 실패");
        dataFile.close();
        return false;
    }

    dataFile.close();
    return true;
}

bool SDManager::play_touch_sound() {
    if (!initialized && !init(cs_pin)) {
        return false;
    }
    return play_sound(TOUCH_SOUND_FILE);
}

bool SDManager::play_sound(const char* filename) {
    if (!initialized || !audio_out) return false;
    
    if (!SD.exists(filename)) {
        handle_error("오디오 파일이 존재하지 않음");
        return false;
    }

    cleanup_audio();

    // 새로운 파일 열기
    current_file = new AudioFileSourceSD(filename);
    if (!current_file->isOpen()) {
        handle_error("오디오 파일 열기 실패");
        cleanup_audio();
        return false;
    }

    // WAV 재생 시작
    current_wav = new AudioGeneratorWAV();
    if (!current_wav->begin(current_file, audio_out)) {
        handle_error("WAV 재생 시작 실패");
        cleanup_audio();
        return false;
    }

    return true;
}

void SDManager::cleanup_audio() {
    if (current_wav) {
        if (current_wav->isRunning()) {
            current_wav->stop();
        }
        delete current_wav;
        current_wav = nullptr;
    }
    if (current_file) {
        delete current_file;
        current_file = nullptr;
    }
}

void SDManager::handle_audio() {
    if (current_wav && current_wav->isRunning()) {
        if (!current_wav->loop()) {
            cleanup_audio();
        }
    }
}

void SDManager::close() {
    cleanup_audio();  // current_wav와 current_file을 정리
    
    if (audio_out) {
        delete audio_out;
        audio_out = nullptr;
    }
    
    i2s_driver_uninstall(I2S_NUM_0);
    SD.end();
    initialized = false;
    State::isSdCardInitialized = false;
}
