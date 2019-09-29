#include <cstdlib>
#include <string>

#include "CO2Sensor.h"

#include "Nixie.h"

#include "CO2Monitor.h"

CO2Monitor::CO2Monitor(bool presistant)
    : AbstractGUIState(), sensor{}, presistant(presistant) {}

void CO2Monitor::setSensor(CO2Sensor *sensor) { this->sensor = sensor; }

void CO2Monitor::enter(InterfaceButton &btn, Nixie *indicators,
                       CFastLED *leds) {
  if (sensor && presistant) {
    prevus_pdate_period = sensor->getUpdateInterval();
    sensor->setUpdateInterval(fast_update_period);
    sensor->setUpdateCallback(
        std::bind(&CO2Monitor::updateDisplay, this, std::placeholders::_1));
  }
  AbstractGUIState::enter(btn, indicators, leds);

  updateDisplay(sensor ? sensor->getCO2Level() : -1);
}

void CO2Monitor::leave() {
  if (sensor && presistant) {
    sensor->setUpdateCallback(nullptr);
    sensor->setUpdateInterval(prevus_pdate_period);
  }
  AbstractGUIState::leave();
}

void CO2Monitor::updateDisplay(int PPM) {
  std::string buf(Nixie::clear_indicators);
  if (PPM >= 0) {
    std::snprintf(buf.data(), buf.size() + 1, "% 6d", PPM);
  }
  indicators->setValue(buf);
}
