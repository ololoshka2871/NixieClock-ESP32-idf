#ifndef _QUICK_RETURN_TO_CLOCK_
#define _QUICK_RETURN_TO_CLOCK_

#include "AbstractGUIStateTransition.h"

struct QuickTransition : public AbstractGUIStateTransition {
  QuickTransition(AbstractGUIState *targetState)
      : AbstractGUIStateTransition{targetState} {}

  const char *getLOG_TAG() const override { return LOG_TAG; }

private:
  static constexpr char LOG_TAG[] = "QuickTransition";
};

#endif /* _QUICK_RETURN_TO_CLOCK_ */
