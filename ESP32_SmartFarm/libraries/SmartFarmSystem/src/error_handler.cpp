#include <Arduino.h>
#include <error_handler.h>
#include <State.h>
#include <SD.h>

void handle_error(const char* message) {
    // 시리얼 출력
    Serial.print("오류: ");
    Serial.println(message);
    
    // SD 카드에 로깅
    if (State::isSdCardInitialized) {
        File errorLog = SD.open("/error_log.txt", FILE_APPEND);
        if (errorLog) {
            char timestamp[32];
            snprintf(timestamp, sizeof(timestamp), "[%lu] ", millis());
            errorLog.write((uint8_t*)timestamp, strlen(timestamp));
            errorLog.println(message);
            errorLog.close();
        }
    }
    
    // TODO: UI에 에러 표시 구현
}





