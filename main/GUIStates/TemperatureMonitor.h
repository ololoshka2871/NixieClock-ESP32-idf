#ifndef _TEMPERATURE_MONITOR_H_
#define _TEMPERATURE_MONITOR_H_

#include "AbstractGUIState.h"

struct TemperatureSensor;

struct TemperatureMonitor : public AbstractGUIState {
  static constexpr std::chrono::seconds fast_update_period =
      std::chrono::seconds(1);

  TemperatureMonitor(bool presistant = false);

  uint64_t idleTimeout() const override {
    return presistant ? 0 : DEFAULT_IDLE_TIMEOUT;
  }
  const char *getLOG_TAG() const override { return LOG_TAG; }

  void enter(InterfaceButton *btn, Nixie *indicators, CFastLED *leds) override;
  void leave() override;

  void setSensor(TemperatureSensor *tsensor);

private:
  static constexpr char LOG_TAG[] = "TemperatureMonitor";

  TemperatureSensor *sensor;
  std::chrono::seconds prevus_pdate_period;
  bool presistant;

  void updateDisplay(float temperature);
};

#endif /* _TEMPERATURE_MONITOR_H_ */
