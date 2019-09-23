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
      esp_timer_start_periodic(timerHandle, button_Check_period_us));
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

bool InterfaceButton::getButtonState() const { return !!gpio_get_level(pin); }

void InterfaceButton::CheckButton() {
  auto newstate = getButtonState();
  auto &el = DefaultEventLoop::instance();
  if (active_counter) {
    // pushed
    if (newstate != active_level) {
      // just released
      el.postEvent(eventGroupName.c_str(), Release, 1);
      if (active_counter < long_push_period) {
        el.postEvent(eventGroupName.c_str(), Click, 1);
      }
      active_counter = 0;
    } else {
      // continue push
      ++active_counter;
      if (active_counter == long_push_period) {
        el.postEvent(eventGroupName.c_str(), LongPush, 1);
      }
    }
  } else if (newstate == active_level) {
    // just pushed
    active_counter = 1;
    el.postEvent(eventGroupName.c_str(), Push, 1);
  }
}
