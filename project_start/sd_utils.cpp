#include "sd_utils.h"

void init_sd() {
    if (!SD.begin(SD_CS)) {
        Serial.println("SD 카드 초기화 실패");
    } else {
        Serial.println("SD 카드 초기화 성공");
    }
}

void play_touch_sound() {
    File soundFile = SD.open("touchsound.wav");
    if (soundFile) {
        Serial.println("터치 사운드 재생 시작");
        while (soundFile.available()) {
            Serial.write(soundFile.read());
        }
        soundFile.close();
        Serial.println("터치 사운드 재생 완료");
    } else {
        Serial.println("터치 사운드 파일 없음");
    }
}
