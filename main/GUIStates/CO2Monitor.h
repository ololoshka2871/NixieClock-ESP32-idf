#ifndef _CO2_MONITOR_H_
#define _CO2_MONITOR_H_

#include <chrono>

#include "AbstractGUIState.h"

struct CO2Sensor;

struct CO2Monitor : public AbstractGUIState {
  static constexpr std::chrono::seconds fast_update_period =
      std::chrono::seconds(5);

  CO2Monitor(bool presistant = false);

  uint64_t idleTimeout() const override {
    return presistant ? 0 : DEFAULT_IDLE_TIMEOUT;
  }
  const char *getLOG_TAG() const override { return LOG_TAG; }

  void setSensor(CO2Sensor *sensor);

  void enter(InterfaceButton *btn, Nixie *indicators, CFastLED *leds) override;

  void leave() override;

private:
  static constexpr char LOG_TAG[] = "CO2Monitor";

  CO2Sensor *sensor;
  std::chrono::seconds prevus_pdate_period;
  bool presistant;

  void updateDisplay(int CO2Level);
};

#endif /* _CO2_MONITOR_H_ */
