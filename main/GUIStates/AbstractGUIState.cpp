#include <functional>

#include <esp_log.h>

#include "InterfaceButton.h"

#include "AbstractGUIState.h"

AbstractGUIState::AbstractGUIState()
    : clickTransition{}, LongPushTransition{}, IdleTransition{},
      idleTimer(std::bind(&AbstractGUIState::onIdleTimerExpired, this)) {}

void AbstractGUIState::enter(InterfaceButton *btn, Nixie *indicators,
                             CFastLED *leds) {
  ESP_LOGD(getLOG_TAG(), "enter()");

  this->indicators = indicators;
  this->leds = leds;

  btn->resetCallbacks();
  if (clickTransition) {
    btn->onClick([this](InterfaceButton::eventID id, gpio_num_t pin) {
      ESP_LOGI(getLOG_TAG(), "Click");
      leave();
      clickTransition->Transit(this->indicators, this->leds);
    });
  }
  if (LongPushTransition) {
    btn->onLongPush([this](InterfaceButton::eventID id, gpio_num_t pin) {
      ESP_LOGI(getLOG_TAG(), "onLongPush");
      leave();
      LongPushTransition->Transit(this->indicators, this->leds);
    });
  }
  if (idleTimeout() != 0) {
    startIdleTimer();

    btn->onPush([this](InterfaceButton::eventID id, gpio_num_t pin) {
      ESP_LOGI(getLOG_TAG(), "stopIdleTimer()");
      stopIdleTimer();
    });
    btn->onRelease([this](InterfaceButton::eventID id, gpio_num_t pin) {
      ESP_LOGI(getLOG_TAG(), "startIdleTimer()");
      startIdleTimer();
    });
  }
}

void AbstractGUIState::leave() {
  ESP_LOGI(getLOG_TAG(), "leave()");
  if (idleTimer.isRunning()) {
    stopIdleTimer();
  }
}

void AbstractGUIState::onIdleTimerExpired() {
  ESP_LOGW(getLOG_TAG(),
           "IdleTimerExpired, running idle transaction (IdleTransition=%X)",
           (size_t)IdleTransition.get());
  if (IdleTransition) {
    leave();
    IdleTransition->Transit(indicators, leds);
  }
}

const char *AbstractGUIState::getLOG_TAG() const { return LOG_TAG; }

void AbstractGUIState::startIdleTimer() {
  idleTimer.start(idleTimeout(), false);
}

void AbstractGUIState::stopIdleTimer() { idleTimer.stop(); }
