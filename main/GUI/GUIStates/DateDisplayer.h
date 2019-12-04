#ifndef _DATE_DISPLAY_H_
#define _DATE_DISPLAY_H_

#include "AbstractGUIState.h"

struct DateDisplay : public AbstractGUIState {
  uint64_t idleTimeout() const override { return DEFAULT_IDLE_TIMEOUT; }
  const char *getLOG_TAG() const override { return LOG_TAG; }

  void enter(ctl::InterfaceButton *btn, Nixie *indicators,
             CFastLED *leds) override;

private:
  static constexpr char LOG_TAG[] = "DateDisplay";
};

#endif /* _DATE_DISPLAY_H_ */
