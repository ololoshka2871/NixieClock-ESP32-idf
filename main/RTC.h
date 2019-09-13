#ifndef _RTC_H_
#define _RTC_H_

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#include <atomic>
#include <functional>
#include <thread>

#include "ds1307.h"

struct RTCManager {
  using onTimeUpdated = std::function<void(std::string)>;

  static constexpr char LOG_TAG[] = "RTCManager";

  // int tm_year;			/* Year	- 1900.  */
  static constexpr int tm_fix_value = 1900;

  static constexpr void fix_tm(tm &_tm) { _tm.tm_year -= tm_fix_value; }
  static constexpr void unfix_tm(tm &_tm) { _tm.tm_year += tm_fix_value; }

  RTCManager(uint8_t rtc_addr = DS1307_ADDR);
  ~RTCManager();
  RTCManager &loadTime();
  RTCManager &setupRTC(const std::time_t &dest_time = 0);
  RTCManager &begin();
  RTCManager &setCallback(const onTimeUpdated &onTimeUpdated);

public:
  void enable_1s_interrupt();

private:
  SemaphoreHandle_t rtc_sem;
  std::thread *update_thread;
  std::atomic<bool> exitflag;
  onTimeUpdated cb;
  i2c_dev_t ds1307_dev;
  uint32_t sync_counter;

  void register_rtc_interrupt();
  void load_clock();

  static void thread_func(RTCManager *_this);
};

#endif /*_RTC_H_*/
