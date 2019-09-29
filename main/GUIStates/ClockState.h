#ifndef _CLOCK_STATE_H_
#define _CLOCK_STATE_H_

#include "AbstractGUIState.h"

struct ClockState : public AbstractGUIState {

  uint64_t idleTimeout() const override { return 0; }
  const char *getLOG_TAG() const override { return LOG_TAG; }

private:
  static constexpr char LOG_TAG[] = "ClockState";
};

#endif /* _CLOCK_STATE_H_ */
