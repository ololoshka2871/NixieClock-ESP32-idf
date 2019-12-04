#ifndef _INITIAL_TRANSITION_H_
#define _INITIAL_TRANSITION_H_

#include "AbstractGUIStateTransition.h"

struct InitialTransition : public AbstractGUIStateTransition {
  InitialTransition(AbstractGUIState *targetState);

  void Transit(Nixie *indicators, CFastLED *leds) override;

  const char *getLOG_TAG() const { return LOG_TAG; }

private:
  static constexpr char LOG_TAG[] = "InitialTransition";
};

#endif /* _INITIAL_TRANSITION_H_ */
