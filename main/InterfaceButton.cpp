#include <esp_log.h>
#include <esp_timer.h>

#include "DefaultEventLoop.h"

#include "InterfaceButton.h"

using namespace ctl;

uint InterfaceButton::registredButtons = 0;

struct InterfaceButton::context {
  context(gpio_num_t gpio, bool active_level)
      : pin(gpio),
        eventGroupName("IB"), callbacks{}, active_level{active_level} {}

  gpio_num_t pin;
  std::string eventGroupName;
  event_cb_t callbacks[SIZE];
  esp_timer_handle_t timerHandle;
  check_period_t active_counter;
  bool active_level;
};

void _cb(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id,
         void *event_data) {
  (void)event_base;
  (void)event_data;

  InterfaceButton *_this = static_cast<InterfaceButton *>(event_handler_arg);
  _this->dispatchEvent(static_cast<InterfaceButton::eventID>(event_id));
}

InterfaceButton::InterfaceButton(gpio_num_t gpio, bool active_level)
    : ctx{std::make_unique<context>(gpio, active_level)} {
  //: pin(gpio), eventGroupName("IB"), callbacks{}, active_level{active_level} {
  ctx->eventGroupName += "registredButtons";
  registredButtons++;
}

InterfaceButton::~InterfaceButton() {
  esp_timer_stop(ctx->timerHandle);

  auto &eventLoop = esp::DefaultEventLoop::instance();
  for (int id = eventID::Push; id < eventID::SIZE; ++id) {
    eventLoop.unRegisterHandler(ctx->eventGroupName.c_str(), id, _cb);
  }
}

void InterfaceButton::begin() {
  auto &eventLoop = esp::DefaultEventLoop::instance();

  for (int id = eventID::Push; id < eventID::SIZE; ++id) {
    eventLoop.registerHandler(ctx->eventGroupName.c_str(), id, _cb, this);
  }

  gpio_reset_pin(ctx->pin);
  gpio_set_direction(ctx->pin, GPIO_MODE_INPUT);
  gpio_set_pull_mode(ctx->pin, GPIO_FLOATING);

  const esp_timer_create_args_t timer_args{
      [](void *arg) {
        auto _this = static_cast<InterfaceButton *>(arg);
        _this->CheckButton();
      },
      this, ESP_TIMER_TASK, ctx->eventGroupName.c_str()};

  ESP_ERROR_CHECK(esp_timer_create(&timer_args, &ctx->timerHandle));
  ESP_ERROR_CHECK(
      esp_timer_start_periodic(ctx->timerHandle, buttonCheckPeriod.count()));
}

void InterfaceButton::dispatchEvent(eventID id) {
  if (ctx->callbacks[id]) {
    ctx->callbacks[id](id, ctx->pin);
  }
}

InterfaceButton &InterfaceButton::onPush(InterfaceButton::event_cb_t &&cb) {
  ctx->callbacks[Push] = std::move(cb);
  return *this;
}

InterfaceButton &InterfaceButton::onRelease(InterfaceButton::event_cb_t &&cb) {
  ctx->callbacks[Release] = std::move(cb);
  return *this;
}

InterfaceButton &InterfaceButton::onClick(InterfaceButton::event_cb_t &&cb) {
  ctx->callbacks[Click] = std::move(cb);
  return *this;
}

InterfaceButton &InterfaceButton::onLongPush(InterfaceButton::event_cb_t &&cb) {
  ctx->callbacks[LongPush] = std::move(cb);
  return *this;
}

InterfaceButton &InterfaceButton::resetCallbacks() {
  for (int i = 0; i < eventID::SIZE; ++i) {
    ctx->callbacks[i] = nullptr;
  }
  return *this;
}

std::chrono::seconds InterfaceButton::longPushTime() const {
  return std::chrono::duration_cast<std::chrono::seconds>(longPushPeriod);
}

void InterfaceButton::dumpCallbacks() const {
  for (int i = 0; i < eventID::SIZE; ++i) {
    ESP_LOGI(LOG_TAG, "callbacks[%d] = %s", i,
             ctx->callbacks[i] ? "present" : "reseted");
  }
}

bool InterfaceButton::getButtonState() const {
  return !!gpio_get_level(ctx->pin);
}

void InterfaceButton::CheckButton() {
  auto newstate = getButtonState();
  auto &el = esp::DefaultEventLoop::instance();
  if (ctx->active_counter.count()) {
    // pushed
    if (newstate != ctx->active_level) {
      // just released
      el.postEvent(ctx->eventGroupName.c_str(), Release, 1);
      if (ctx->active_counter < longPushPeriod) {
        el.postEvent(ctx->eventGroupName.c_str(), Click, 1);
      }
      ctx->active_counter = check_period_t{0};
    } else {
      // continue push
      ++ctx->active_counter;
      if (ctx->active_counter == longPushPeriod) {
        el.postEvent(ctx->eventGroupName.c_str(), LongPush, 1);
      }
    }
  } else if (newstate == ctx->active_level) {
    // just pushed
    ctx->active_counter = check_period_t{1};
    el.postEvent(ctx->eventGroupName.c_str(), Push, 1);
  }
}
