#include "Timer.h"

esp::Timer::Timer(const timer_cb_t &cb) : cb(cb), timerHandle{}, running{IDLE} {
  const esp_timer_create_args_t timer_args{[](void *arg) {
                                             auto _this =
                                                 static_cast<Timer *>(arg);
                                             _this->timerExpired();
                                           },
                                           this, ESP_TIMER_TASK, nullptr};
  ESP_ERROR_CHECK(esp_timer_create(&timer_args, &timerHandle));
}

void esp::Timer::start(uint64_t timeout_us, bool periodic) {
  if (isRunning()) {
    stop();
  }

  if (periodic) {
    running = PERIODICAL;
    ESP_ERROR_CHECK(esp_timer_start_periodic(timerHandle, timeout_us));
  } else {
    running = ONESHOT;
    ESP_ERROR_CHECK(esp_timer_start_once(timerHandle, timeout_us));
  }
}

void esp::Timer::stop() { ESP_ERROR_CHECK(esp_timer_stop(timerHandle)); }

void esp::Timer::timerExpired() {
  if (running == ONESHOT) {
    running = IDLE;
  }
  if (cb) {
    cb();
  }
}
