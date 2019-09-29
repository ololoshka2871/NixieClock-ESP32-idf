#include <cmath>
#include <limits>

#include "TemperatureSensor.h"

#include "Nixie.h"

#include "TemperatureMonitor.h"

TemperatureMonitor::TemperatureMonitor(bool presistant)
    : AbstractGUIState(), presistant(presistant) {}

void TemperatureMonitor::setSensor(TemperatureSensor *tsensor) {
  this->sensor = tsensor;
}

void TemperatureMonitor::enter(InterfaceButton &btn, Nixie *indicators,
                               CFastLED *leds) {
  if (sensor && presistant) {
    prevus_pdate_period = sensor->getUpdateInterval();
    sensor->setUpdateInterval(fast_update_period);
    sensor->setUpdateCallback(std::bind(&TemperatureMonitor::updateDisplay,
                                        this, std::placeholders::_1));
  }
  AbstractGUIState::enter(btn, indicators, leds);

  updateDisplay(sensor ? sensor->temperature()
                       : std::numeric_limits<float>::quiet_NaN());
}

void TemperatureMonitor::leave() {
  if (sensor && presistant) {
    sensor->setUpdateCallback(nullptr);
    sensor->setUpdateInterval(prevus_pdate_period);
  }
  AbstractGUIState::leave();
}

void TemperatureMonitor::updateDisplay(float temperature) {
  std::string buf(Nixie::clear_indicators);
  if (!std::isnan(temperature)) {
    std::snprintf(buf.data(), buf.size() + 1, "% 6d",
                  (int)std::floor(temperature * 100));
  }
  indicators->setValue(buf);
}
