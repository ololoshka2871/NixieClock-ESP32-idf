#ifndef _ABSTRACT_GUI_STATE_H_
#define _ABSTRACT_GUI_STATE_H_

#include <functional>
#include <memory>

#include <Timer.h>

#include "AbstractGUIStateTransition.h"
#include "GUI.h"
#include "InterfaceButton.h"

#include "InterfaceButton.h"

class Nixie;
class CFastLed;

struct AbstractGUIState {

  static constexpr uint64_t DEFAULT_IDLE_TIMEOUT = 5_s;

  AbstractGUIState()
      : clickTransition{}, LongPushTransition{}, IdleTransition{},
        idleTimer(std::bind(&AbstractGUIState::onIdleTimerExpired, this)) {}

  virtual void enter(InterfaceButton &btn, Nixie *indicators, CFastLED *leds) {
    ESP_LOGI(getLOG_TAG(), "enter()");

    this->indicators = indicators;
    this->leds = leds;

    btn.resetCallbacks();
    if (clickTransition) {
      btn.onClick([this](InterfaceButton::eventID id, gpio_num_t pin) {
        ESP_LOGI(getLOG_TAG(), "Click");
        leave();
        clickTransition->Transit(this->indicators, this->leds);
      });
    }
    if (LongPushTransition) {
      btn.onLongPush([this](InterfaceButton::eventID id, gpio_num_t pin) {
        ESP_LOGI(getLOG_TAG(), "onLongPush");
        leave();
        LongPushTransition->Transit(this->indicators, this->leds);
      });
    }
    if (idleTimeout()) {
      idleTimer.start(idleTimeout(), false);
    }
  }

  virtual void leave() {
    ESP_LOGI(getLOG_TAG(), "leave()");
    if (idleTimer.isRunning()) {
      idleTimer.stop();
    }
  }

  virtual uint64_t idleTimeout() const = 0;

  std::shared_ptr<AbstractGUIStateTransition> clickTransition,
      LongPushTransition, IdleTransition;

protected:
  void virtual onIdleTimerExpired() {
    ESP_LOGW(getLOG_TAG(),
             "IdleTimerExpired, running idle transaction (IdleTransition=%X)",
             (size_t)IdleTransition.get());
    if (IdleTransition) {
      leave();
      IdleTransition->Transit(indicators, leds);
    }
  }

  esp::Timer idleTimer;

  virtual const char *getLOG_TAG() const { return LOG_TAG; }

private:
  static constexpr char LOG_TAG[] = "AbstractGUIState";

protected:
  Nixie *indicators;
  CFastLED *leds;
};

#endif /* _ABSTRACT_GUI_STATE_H_ */
