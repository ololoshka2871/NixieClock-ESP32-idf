#include "cstdio"

#include <esp_log.h>

#include "SoftAP.h"

nw::SoftAP::~SoftAP() {
  esp_log_write(ESP_LOG_INFO, LOG_TAG, __PRETTY_FUNCTION__);
}

void nw::SoftAP::start() {
  esp_log_write(ESP_LOG_INFO, LOG_TAG, __PRETTY_FUNCTION__);
}
