#ifndef _CO2_MONITOR_H_
#define _CO2_MONITOR_H_

#include "AbstractGUIState.h"

struct CO2Monitor : public AbstractGUIState {

  CO2Monitor(bool presistant = false)
      : AbstractGUIState(), presistant(presistant) {}

  uint64_t idleTimeout() const override {
    return presistant ? 0 : DEFAULT_IDLE_TIMEOUT;
  }

  const char *getLOG_TAG() const override { return LOG_TAG; }

private:
  static constexpr char LOG_TAG[] = "CO2Monitor";

  bool presistant;
};

#endif /* _CO2_MONITOR_H_ */
