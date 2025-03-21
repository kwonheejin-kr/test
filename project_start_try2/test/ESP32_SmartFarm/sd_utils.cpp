#include "sd_utils.h"

void init_sd() {
    if (!SD.begin(SD_CS)) {
        Serial.println("❌ SD 카드 초기화 실패: SD 카드가 삽입되지 않았거나 손상됨");
    } else {
        Serial.println("✅ SD 카드 초기화 성공");
    }
}

void save_heart_data(float bpm) {
    if (!SD.exists("/heartDATA")) {
        SD.mkdir("/heartDATA");
    }

    File heartDataFile = SD.open("/heartDATA/heart_data.txt", FILE_APPEND);
    if (heartDataFile) {
        if (heartDataFile.size() > 10000) {  // 10KB 이상이면 새 파일 생성
            heartDataFile.close();
            String newFileName = "/heartDATA/heart_data_" + String(millis()) + ".txt";
            heartDataFile = SD.open(newFileName, FILE_WRITE);
            Serial.println("📂 새 데이터 파일 생성: " + newFileName);
        }

        heartDataFile.printf("%lu, %.1f\n", millis(), bpm);
        heartDataFile.close();
    } else {
        Serial.println("❌ SD 카드에 데이터 저장 실패!");
    }
}

void play_touch_sound() {
    File soundFile = SD.open("touchsound.wav");
    if (soundFile) {
        Serial.println("🔊 터치 사운드 재생 시작");

        uint8_t buffer[512];  // 512바이트 버퍼 사용
        while (soundFile.available()) {
            size_t bytesRead = soundFile.read(buffer, sizeof(buffer));
            Serial.write(buffer, bytesRead);
        }

        soundFile.close();
        Serial.println("✅ 터치 사운드 재생 완료");
    } else {
        Serial.println("❌ 터치 사운드 파일 없음");
    }
}
