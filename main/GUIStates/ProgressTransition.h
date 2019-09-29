#ifndef _PROGRESS_TRANSITION_H_
#define _PROGRESS_TRANSITION_H_

#include "AbstractGUIStateTransition.h"

struct ProgressTransition : public AbstractGUIStateTransition {
  ProgressTransition(AbstractGUIState *targetState)
      : AbstractGUIStateTransition{targetState} {}

  const char *getLOG_TAG() const { return LOG_TAG; }

private:
  static constexpr char LOG_TAG[] = "ProgressTransition";
};

#endif /* _PROGRESS_TRANSITION_H_ */
