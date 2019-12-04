#include "DefaultEventLoop.h"

using namespace esp;

DefaultEventLoop *DefaultEventLoop::inst = nullptr;

DefaultEventLoop &DefaultEventLoop::instance() {
  if (inst == nullptr) {
    inst = new DefaultEventLoop;
  }
  return *inst;
}

esp_err_t DefaultEventLoop::registerHandler(esp_event_base_t event_base,
                                            int32_t event_id,
                                            esp_event_handler_t event_handler,
                                            void *event_handler_arg) {
  return esp_event_handler_register(event_base, event_id, event_handler,
                                    event_handler_arg);
}

esp_err_t
DefaultEventLoop::unRegisterHandler(esp_event_base_t event_base,
                                    int32_t event_id,
                                    esp_event_handler_t event_handler) {
  return esp_event_handler_unregister(event_base, event_id, event_handler);
}

esp_err_t DefaultEventLoop::postEvent(esp_event_base_t event_base,
                                      int32_t event_id, void *event_data,
                                      size_t event_data_size,
                                      TickType_t ticks_to_wait) {
  return esp_event_post(event_base, event_id, event_data, event_data_size,
                        ticks_to_wait);
}

esp_err_t DefaultEventLoop::postEvent(esp_event_base_t event_base,
                                      int32_t event_id,
                                      TickType_t ticks_to_wait) {
  return esp_event_post(event_base, event_id, nullptr, 0, ticks_to_wait);
}

DefaultEventLoop::DefaultEventLoop() {
  ESP_ERROR_CHECK(esp_event_loop_create_default());
}
