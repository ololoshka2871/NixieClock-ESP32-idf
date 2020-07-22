#include <chrono>

#include <esp_log.h>

#include "String_format.h"

#include "GUI/GUI.h"
#include "RTC.h"

#include "VSensors.h"
#include "sensors/CO2Sensor.h"
#include "sensors/TemperatureSensor.h"

#include "VirtuinoJsonServer.value_router.h"

static constexpr char LOG_TAG[] = "VirtuinoJsonServer.value_router";

CO2Sensor *VirtuinoSensorsHolder::co2Sensor;
TemperatureSensor *VirtuinoSensorsHolder::temperatureSensor;
VSensors *VirtuinoSensorsHolder::vsensors;

template <size_t channel> json getVoltage() {
  if (VirtuinoSensorsHolder::vsensors) {
    return json::string_t{
        to_string(VirtuinoSensorsHolder::vsensors->getChannelVoltage(channel))};
  }
  return json::string_t{"0"};
}

const std::vector<valuesAccessors> value_router{
    // V0 - clock
    {[]() {
       int64_t miliseconds =
           std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::system_clock::now().time_since_epoch())
               .count();
       return json::string_t{to_string(miliseconds)};
     },
     [](const json &newval) {
       if (newval.is_string()) {
         uint64_t t_ms;
         auto s = newval.get<std::string>();
         auto r = sscanf(s.c_str(), "%lld", &t_ms);
         if (r == 1) {
           timeval tv{static_cast<time_t>(t_ms / 1000),
                      static_cast<__suseconds_t>((t_ms % 1000) * 1000)};
           RTCManager::instance()->setupRTC(tv);
         } else {
           ESP_LOGE(LOG_TAG, "parsing %s, res = %d", s.c_str(), r);
         }
       }
     }},
    // V1 - color
    {[]() {
       union {
         int32_t _signed;
         uint32_t _unsigned;
       } v;
       v._unsigned = GUI::getClockBGColor();
       v._unsigned = v._unsigned | (0xFF << 24);
       return json::string_t{to_string(v._signed)};
     },
     [](const json &value) {
       if (value.is_string()) {
         union {
           int32_t _signed;
           uint32_t _unsigned;
         } v;
         auto s = value.get<std::string>();
         auto r = sscanf(s.c_str(), "%d", &v._signed);
         if (r == 1) {
           // see https://stackoverflow.com/a/38570206
           v._unsigned = v._unsigned & ((1 << 24) - 1);
           GUI::setClockBGColor(v._unsigned);
         } else {
           ESP_LOGE(LOG_TAG, "failed to get int from %s", s.c_str());
         }
       }
     }},
    // V2 - CO2 level
    {[]() {
       if (VirtuinoSensorsHolder::co2Sensor) {
         return json::string_t{
             to_string(VirtuinoSensorsHolder::co2Sensor->getCO2Level())};
       }
       return json::string_t{"0"};
     },
     // calibrate CO2 sensor
     [](const json &newval) {
       if (newval.is_string()) {
         int32_t method;
         auto s = newval.get<std::string>();
         auto r = sscanf(s.c_str(), "%d", &method);

         if (r == 1 && VirtuinoSensorsHolder::co2Sensor) {
           VirtuinoSensorsHolder::co2Sensor->startCalibration(method);
         } else {
           ESP_LOGE(LOG_TAG, "\"%s\" is not V2 applicatable value", s.c_str());
         }
       }
     }},
    // V3 - Temperature
    {[]() {
       if (VirtuinoSensorsHolder::temperatureSensor) {
         return json::string_t{to_string(
             VirtuinoSensorsHolder::temperatureSensor->temperature())};
       }
       return json::string_t{"0"};
     },
     nullptr},
    // V4 - voltage1
    {getVoltage<0>, nullptr},
    // V5 - voltage2
    {getVoltage<1>, nullptr},
    // V6 - voltage3
    {getVoltage<2>, nullptr},
};
