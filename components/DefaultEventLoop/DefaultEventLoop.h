#ifndef _DEFAULT_EVENT_LOOP_
#define _DEFAULT_EVENT_LOOP_

#include <esp_event.h>

namespace esp {

struct DefaultEventLoop {
  static DefaultEventLoop &instance();

  esp_err_t registerHandler(esp_event_base_t event_base, int32_t event_id,
                            esp_event_handler_t event_handler,
                            void *event_handler_arg);
  esp_err_t unRegisterHandler(esp_event_base_t event_base, int32_t event_id,
                              esp_event_handler_t event_handler);

  esp_err_t postEvent(esp_event_base_t event_base, int32_t event_id,
                      void *event_data, size_t event_data_size,
                      TickType_t ticks_to_wait);
  esp_err_t postEvent(esp_event_base_t event_base, int32_t event_id,
                      TickType_t ticks_to_wait);

private:
  static DefaultEventLoop *inst;

  DefaultEventLoop();
};

} // namespace esp

#endif /*_DEFAULT_EVENT_LOOP_*/
