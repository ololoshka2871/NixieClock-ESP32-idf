#include <cmath>
#include <cstdlib>
#include <string>
#include <tuple>

#include <esp_log.h>

#include "String_format.h"

#include "CO2Sensor.h"

#include "FastLED.h"
#include "Nixie.h"

#include "CO2Monitor.h"

template <typename T> static std::string print_color(const T &hsvcolor) {
  return format("CRGB{%X, %X, %X}", hsvcolor.r, hsvcolor.g, hsvcolor.b);
}

static std::tuple<int, CRGB, CRGB> ledsToShow(int level) {
  static constexpr int minimalCO2Level = 400;
  static constexpr int maximalCO2Level = 2000;

  if (level < minimalCO2Level) {
    return std::tuple<int, CRGB, CRGB>(1, CRGB::Green, CRGB::Black);
  } else if (level > maximalCO2Level) {
    level -= maximalCO2Level - 1;
    const auto V = 30 * std::log(level);
    CRGB color{CHSV{HUE_RED, 255,
                    (V < 0xff) ? (uint8_t)(0xff - (uint8_t)V) : (uint8_t)0}};

    return std::tuple<int, CRGB, CRGB>(5, color, color);
  } else {
    // k * (maximalCO2Level - minimalCO2Level) = HUE_RED - HUE_GREEN
    static constexpr float k =
        (HUE_RED - HUE_GREEN) / (float)(maximalCO2Level - minimalCO2Level);
    static constexpr int level_pre_led =
        (maximalCO2Level - minimalCO2Level) / 5;

    level -= minimalCO2Level;
    uint8_t H = HUE_GREEN + (int8_t)std::round(k * level);
    auto count = level / level_pre_led;
    auto ov = level - (count * level_pre_led);
    uint8_t V = ((int)0xff * ov) / level_pre_led;

    CRGB fillColor{CHSV{H, (uint8_t)0xff, (uint8_t)0xff}};
    CRGB addColor{CHSV{H, (uint8_t)0xff, V}};
    ESP_LOGD("ledsToShow()", "(%d, %s, %s)", count,
             print_color(fillColor).c_str(), print_color(addColor).c_str());

    return std::tuple<int, CRGB, CRGB>(count, fillColor, addColor);
  }
}

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

void CO2Monitor::updateDisplay(int CO2Level) {
  indicators->setValue(CO2Level >= 0 ? format("% 6d", CO2Level)
                                     : Nixie::clear_indicators);

  leds->clear();

  const auto led_to_show = ledsToShow(CO2Level);
  auto _leds = leds->leds();
  int i = 0;

  for (; i <= std::get<0>(led_to_show); ++i) {
    _leds[5 - i] = std::get<1>(led_to_show);
  }
  if (i < 5) {
    _leds[5 - i] = std::get<2>(led_to_show);
    ++i;
  }
  for (; i < 6; ++i) {
    _leds[5 - i] = CRGB::Black;
  }

  leds->show();
}
