#ifndef SYSTEM_MONITOR_H
#define SYSTEM_MONITOR_H

#include <Arduino.h>
#include "lvgl.h"
#include "ui_components.h"  // ui.h 대신 ui_components.h 사용
#include "State.h"

class SystemMonitor {
private:
    static unsigned long lastUpdate;
    static const size_t MEMORY_WARNING_THRESHOLD = 10000;  // 10KB
    static portMUX_TYPE mux;

public:
    static void init();
    static void update();
    static void checkHeapMemory();
    static void checkPSRAM();
    static void printMemoryInfo();
    static bool checkMemory();
    static bool checkSystemStatus();
    static void logSystemStatus();
    static void log_error(const char* error_msg);
};

#endif





