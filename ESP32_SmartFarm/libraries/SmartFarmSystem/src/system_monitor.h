#ifndef SYSTEM_MONITOR_H
#define SYSTEM_MONITOR_H

#include <Arduino.h>
#include <State.h>
#include <Config.h>
#include "../ui/ui_components.h"

class SystemMonitor {
public:
    static void init();
    static void update();
    
private:
    static unsigned long lastUpdateTime;
};

#endif // SYSTEM_MONITOR_H



