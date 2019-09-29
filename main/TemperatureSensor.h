#ifndef TEMPERATURE_SENSOR_H_
#define TEMPERATURE_SENSOR_H_

#include <atomic>
#include <chrono>
#include <functional>
#include <thread>

#include "ds18x20.h"

struct TemperatureSensor {
  using callback_t = std::function<void(float)>;

  static constexpr char LOG_TAG[] = "TemperatureSensor";

  TemperatureSensor(gpio_num_t OneWirePin,
                    const std::chrono::seconds &updatePeriod_s);
  TemperatureSensor(const TemperatureSensor &) = delete;
  ~TemperatureSensor();

  void setUpdateInterval(const std::chrono::seconds &update_interval);
  std::chrono::seconds getUpdateInterval() const;

  bool begin();
  float temperature() const;

  void setUpdateCallback(const callback_t &cb);

private:
  const gpio_num_t OneWirePin;
  std::chrono::seconds temperature_update_interval;
  std::thread *temperature_reader;
  ds18x20_addr_t sensorAddress;
  std::atomic<float> sensed_temperature;
  std::atomic<bool> exitflag;
  callback_t callback;

  static void thread_func(TemperatureSensor *self);
};

#endif /* TEMPERATURE_SENSOR_H_ */
