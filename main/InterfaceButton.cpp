#include <esp_log.h>
#include <esp_timer.h>

#include "DefaultEventLoop.h"

#include "InterfaceButton.h"

uint InterfaceButton::registredButtons = 0;

void _cb(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id,
         void *event_data) {
  (void)event_base;
  (void)event_data;

  InterfaceButton *_this = static_cast<InterfaceButton *>(event_handler_arg);
  _this->dispatchEvent(static_cast<InterfaceButton::eventID>(event_id));
}

InterfaceButton::InterfaceButton(gpio_num_t gpio, bool active_level)
    : pin(gpio), eventGroupName("IB"), callbacks{}, active_level{active_level} {
  eventGroupName += "registredButtons";
  registredButtons++;
}

InterfaceButton::~InterfaceButton() {
  esp_timer_stop(timerHandle);

  auto &eventLoop = DefaultEventLoop::instance();
  for (int id = eventID::Push; id < eventID::SIZE; ++id) {
    eventLoop.unRegisterHandler(eventGroupName.c_str(), id, _cb);
  }
}

void InterfaceButton::begin() {
  auto &eventLoop = DefaultEventLoop::instance();

  for (int id = eventID::Push; id < eventID::SIZE; ++id) {
    eventLoop.registerHandler(eventGroupName.c_str(), id, _cb, this);
  }

  gpio_reset_pin(pin);
  gpio_set_direction(pin, GPIO_MODE_INPUT);
  gpio_set_pull_mode(pin, GPIO_FLOATING);

  const esp_timer_create_args_t timer_args{
      [](void *arg) {
        auto _this = static_cast<InterfaceButton *>(arg);
        _this->CheckButton();
      },
      this, ESP_TIMER_TASK, eventGroupName.c_str()};

  ESP_ERROR_CHECK(esp_timer_create(&timer_args, &timerHandle));
  ESP_ERROR_CHECK(
      esp_timer_start_periodic(timerHandle, buttonCheckPeriod.count()));
}

void InterfaceButton::dispatchEvent(eventID id) {
  if (callbacks[id]) {
    callbacks[id](id, pin);
  }
}

InterfaceButton &InterfaceButton::onPush(InterfaceButton::event_cb_t &&cb) {
  callbacks[Push] = std::move(cb);
  return *this;
}

InterfaceButton &InterfaceButton::onRelease(InterfaceButton::event_cb_t &&cb) {
  callbacks[Release] = std::move(cb);
  return *this;
}

InterfaceButton &InterfaceButton::onClick(InterfaceButton::event_cb_t &&cb) {
  callbacks[Click] = std::move(cb);
  return *this;
}

InterfaceButton &InterfaceButton::onLongPush(InterfaceButton::event_cb_t &&cb) {
  callbacks[LongPush] = std::move(cb);
  return *this;
}

InterfaceButton &InterfaceButton::resetCallbacks() {
  for (int i = 0; i < eventID::SIZE; ++i) {
    callbacks[i] = nullptr;
  }
  return *this;
}

std::chrono::seconds InterfaceButton::longPushTime() const {
  return std::chrono::duration_cast<std::chrono::seconds>(longPushPeriod);
}

void InterfaceButton::dumpCallbacks() const {
  for (int i = 0; i < eventID::SIZE; ++i) {
    ESP_LOGI(LOG_TAG, "callbacks[%d] = %s", i,
             callbacks[i] ? "present" : "reseted");
  }
}

bool InterfaceButton::getButtonState() const { return !!gpio_get_level(pin); }

void InterfaceButton::CheckButton() {
  auto newstate = getButtonState();
  auto &el = DefaultEventLoop::instance();
  if (active_counter.count()) {
    // pushed
    if (newstate != active_level) {
      // just released
      el.postEvent(eventGroupName.c_str(), Release, 1);
      if (active_counter < longPushPeriod) {
        el.postEvent(eventGroupName.c_str(), Click, 1);
      }
      active_counter = check_period_t{0};
    } else {
      // continue push
      ++active_counter;
      if (active_counter == longPushPeriod) {
        el.postEvent(eventGroupName.c_str(), LongPush, 1);
      }
    }
  } else if (newstate == active_level) {
    // just pushed
    active_counter = check_period_t{1};
    el.postEvent(eventGroupName.c_str(), Push, 1);
  }
}
