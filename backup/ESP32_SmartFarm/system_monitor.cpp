#include "system_monitor.h"
#include <esp_system.h>
#include <esp_heap_caps.h>
#include "error_handler.h"
#include <SD.h>

unsigned long SystemMonitor::lastUpdate = 0;
portMUX_TYPE SystemMonitor::mux = portMUX_INITIALIZER_UNLOCKED;

void SystemMonitor::init() {
    lastUpdate = 0;
    checkSystemStatus();
    printMemoryInfo();
}

void SystemMonitor::update() {
    unsigned long currentMillis = millis();
    
    portENTER_CRITICAL(&mux);
    // 시스템 상태 체크 및 업데이트
    checkSystemStatus();
    portEXIT_CRITICAL(&mux);
}

void SystemMonitor::printMemoryInfo() {
    size_t freeHeap = esp_get_free_heap_size();
    size_t minFreeHeap = esp_get_minimum_free_heap_size();
    
    Serial.printf("Free heap: %u bytes\n", freeHeap);
    Serial.printf("Minimum free heap since boot: %u bytes\n", minFreeHeap);
    
    #ifdef CONFIG_SPIRAM_SUPPORT
    size_t freePSRAM = esp_get_free_internal_heap_size();
    Serial.printf("Free PSRAM: %u bytes\n", freePSRAM);
    #endif
}

bool SystemMonitor::checkMemory() {
    size_t freeHeap = esp_get_free_heap_size();
    if (freeHeap < MEMORY_WARNING_THRESHOLD) {
        log_error("메모리 부족 경고");
        return false;
    }
    return true;
}

bool SystemMonitor::checkSystemStatus() {
    bool status = true;
    
    if (!checkMemory()) {
        status = false;
    }
    
    return status;
}

void SystemMonitor::checkHeapMemory() {
    size_t freeHeap = esp_get_free_heap_size();
    if (freeHeap < MEMORY_WARNING_THRESHOLD) {
        log_error("힙 메모리 부족");
    }
}

void SystemMonitor::checkPSRAM() {
    #ifdef CONFIG_SPIRAM_SUPPORT
    size_t freePSRAM = esp_get_free_internal_heap_size();
    if (freePSRAM < MEMORY_WARNING_THRESHOLD) {
        log_error("PSRAM 메모리 부족");
    }
    #endif
}

void SystemMonitor::logSystemStatus() {
    size_t freeHeap = esp_get_free_heap_size();
    size_t minFreeHeap = esp_get_minimum_free_heap_size();
    
    String statusMsg = "시스템 상태:\n";
    statusMsg += "Free Heap: " + String(freeHeap) + " bytes\n";
    statusMsg += "Min Free Heap: " + String(minFreeHeap) + " bytes\n";
    
    #ifdef CONFIG_SPIRAM_SUPPORT
    size_t freePSRAM = esp_get_free_internal_heap_size();
    statusMsg += "Free PSRAM: " + String(freePSRAM) + " bytes\n";
    #endif
    
    Serial.println(statusMsg);
}

void SystemMonitor::log_error(const char* error_msg) {
    if (!SD.begin()) {
        return;
    }
    
    File logFile = SD.open("/error.log", FILE_APPEND);
    if (logFile) {
        // 타임스탬프 추가
        unsigned long now = millis();
        logFile.print(now);
        logFile.print(": ");
        logFile.println(error_msg);
        logFile.close();
    }
    SD.end();
}



