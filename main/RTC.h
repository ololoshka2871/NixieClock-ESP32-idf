#ifndef _RTC_H_
#define _RTC_H_

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#include <atomic>
#include <functional>
#include <thread>

#include <driver/i2c.h>

struct RTCManager {
  using onTimeUpdated = std::function<void(std::string)>;

  static constexpr uint8_t DEFAULT_DS1307_ADDR = 0xD0;

  RTCManager(uint8_t rtc_addr = DEFAULT_DS1307_ADDR);
  ~RTCManager();
  RTCManager &loadTime();
  RTCManager &begin();
  RTCManager &setCallback(const onTimeUpdated &onTimeUpdated);

private:
  SemaphoreHandle_t rtc_sem;
  std::thread *update_thread;
  std::atomic<bool> exitflag;
  onTimeUpdated cb;
  i2c_config_t i2c;
  uint32_t sync_counter;
  uint8_t clock_i2c_addr;

  void register_rtc_interrupt();
  void load_clock();

  static void thread_func(RTCManager *_this);
};

#endif /*_RTC_H_*/
