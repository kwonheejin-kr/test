#include "error_handler.h"
#include "State.h"
#include <Arduino.h>
#include <SD.h>

void handle_error(const char* message) {
    Serial.print("오류: ");
    Serial.println(message);
    
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
}

void log_error(const char* message) {
    handle_error(message);
}