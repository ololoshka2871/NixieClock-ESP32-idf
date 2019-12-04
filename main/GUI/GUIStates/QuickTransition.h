#ifndef _QUICK_TRANSITION_CLOCK_
#define _QUICK_TRANSITION_CLOCK_

#include "AbstractGUIStateTransition.h"

struct QuickTransition : public AbstractGUIStateTransition {
  QuickTransition(AbstractGUIState *targetState)
      : AbstractGUIStateTransition{targetState} {}

  const char *getLOG_TAG() const override { return LOG_TAG; }

  void Transit(Nixie *indicators, CFastLED *leds) override;

private:
  static constexpr char LOG_TAG[] = "QuickTransition";
};

#endif /* _QUICK_TRANSITION_CLOCK_ */
