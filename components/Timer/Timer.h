#ifndef _TIMER_H_
#define _TIMER_H_

#include <chrono>
#include <functional>

#include <esp_timer.h>

uint64_t constexpr operator"" _s(unsigned long long v) { return v * 1000000; }

namespace esp {

struct Timer {
  using timer_cb_t = std::function<void()>;

  Timer(const timer_cb_t &cb = timer_cb_t());

  void start(uint64_t timeout_us, bool periodic = true);

  void stop();
  bool isRunning() const { return running != IDLE; }

private:
  void timerExpired();

  timer_cb_t cb;
  esp_timer_handle_t timerHandle;

  enum mode { IDLE, ONESHOT, PERIODICAL } running;
};

} // namespace esp

#endif
