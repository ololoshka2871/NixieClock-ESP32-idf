#include <esp_timer.h>

extern "C" unsigned long micros() {
    return esp_timer_get_time();
}

extern "C" unsigned long millis() {
    return esp_timer_get_time() / 1000;
}
