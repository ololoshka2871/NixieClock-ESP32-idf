#ifndef _VIRTUINO_JSON_SERVER_VALUE_ROUTER_H_
#define _VIRTUINO_JSON_SERVER_VALUE_ROUTER_H_

#include <functional>
#include <vector>

#include "nlohmann/json.hpp"

struct CO2Sensor;
struct TemperatureSensor;
struct VSensors;

using json = nlohmann::json;

struct valuesAccessors {
  using getter_t = std::function<json()>;
  using setter_t = std::function<void(const json &)>;

  valuesAccessors(getter_t &&getter, setter_t &&setter)
      : getter{std::move(getter)}, setter{std::move(setter)} {}

  const getter_t getter;
  const setter_t setter;
};

struct VirtuinoSensorsHolder {
  static CO2Sensor *co2Sensor;
  static TemperatureSensor *temperatureSensor;
  static VSensors *vsensors;
};

extern const std::vector<valuesAccessors> value_router;

#endif /* _VIRTUINO_JSON_SERVER_VALUE_ROUTER_H_ */
