#ifndef _TEMPERATURE_MONITOR_H_
#define _TEMPERATURE_MONITOR_H_

#include "AbstractGUIState.h"

struct TemperatureMonitor : public AbstractGUIState {
  TemperatureMonitor(bool presistant = false)
      : AbstractGUIState(), presistant(presistant) {}

  uint64_t idleTimeout() const override {
    return presistant ? 0 : DEFAULT_IDLE_TIMEOUT;
  }
  const char *getLOG_TAG() const override { return LOG_TAG; }

private:
  static constexpr char LOG_TAG[] = "TemperatureMonitor";

  bool presistant;
};

#endif /* _TEMPERATURE_MONITOR_H_ */
