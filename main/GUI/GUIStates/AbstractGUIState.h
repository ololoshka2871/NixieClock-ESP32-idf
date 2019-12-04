#ifndef _ABSTRACT_GUI_STATE_H_
#define _ABSTRACT_GUI_STATE_H_

#include <functional>
#include <memory>

#include <Timer.h>

#include "AbstractGUIStateTransition.h"

class Nixie;
class CFastLed;

namespace ctl {
class InterfaceButton;
}

struct AbstractGUIState {

  static constexpr uint64_t DEFAULT_IDLE_TIMEOUT = 5_s;

  AbstractGUIState();

  virtual void enter(ctl::InterfaceButton *btn, Nixie *indicators,
                     CFastLED *leds);
  virtual void leave();

  virtual uint64_t idleTimeout() const = 0;

  std::shared_ptr<AbstractGUIStateTransition> clickTransition,
      LongPushTransition, IdleTransition;

protected:
  void virtual onIdleTimerExpired();

  esp::Timer idleTimer;

  Nixie *indicators;
  CFastLED *leds;

  virtual const char *getLOG_TAG() const;

  void startIdleTimer();
  void stopIdleTimer();

private:
  static constexpr char LOG_TAG[] = "AbstractGUIState";
};

#endif /* _ABSTRACT_GUI_STATE_H_ */
