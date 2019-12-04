#include <cmath>
#include <limits>

#include <esp_log.h>

#include "String_format.h"

#include "sensors/TemperatureSensor.h"

#include "FastLED.h"
#include "Nixie.h"

#include "TemperatureMonitor.h"

static CRGB temperture2Color(float T, bool Low) {
  static constexpr float minimumTemperature = 0.0f;
  static constexpr float maximumTemperature = 50.0f;

  // k * (maximumTemperature - minimumTemperature) = HUE_RED - HUE_BLUE
  const float k =
      (HUE_RED - HUE_BLUE) / (maximumTemperature - minimumTemperature);

  if (T <= minimumTemperature) {
    return CRGB::Blue;
  } else if (T >= maximumTemperature) {
    return CRGB::Red;
  }

  auto H = HUE_BLUE + (int8_t)std::round(k * T);
  CRGB res{CHSV(H, Low ? 200 : 255, Low ? 100 : 255)};
  return res;
}

TemperatureMonitor::TemperatureMonitor(bool presistant)
    : AbstractGUIState(), presistant(presistant) {}

void TemperatureMonitor::setSensor(TemperatureSensor *tsensor) {
  this->sensor = tsensor;
}

void TemperatureMonitor::enter(ctl::InterfaceButton *btn, Nixie *indicators,
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
  indicators->setValue(
      !std::isnan(temperature)
          ? format(" % 5d", (int)std::round(std::fabs(temperature * 100)))
          : Nixie::clear_indicators);

  auto _leds = leds->leds();

  _leds[1] = _leds[0] = temperture2Color(temperature, true);
  _leds[4] = _leds[3] = _leds[2] = temperture2Color(temperature, false);

  _leds[5] = temperature < 0 ? CRGB::BlueViolet : CRGB::Red;

  leds->show();
}
