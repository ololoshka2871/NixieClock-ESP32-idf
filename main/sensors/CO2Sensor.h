#ifndef _CO2_SENSOR_H_
#define _CO2_SENSOR_H_

#include <atomic>
#include <functional>
#include <thread>

#include <MHZ19.h>

struct CO2Sensor {
  using callback_t = std::function<void(int)>;

  static constexpr char LOG_TAG[] = "CO2Sensor";

  CO2Sensor(
      const uart_port_t uart_num, const gpio_num_t rx, const gpio_num_t tx,
      const std::chrono::seconds &update_interval = std::chrono::minutes(1));

  ~CO2Sensor();

  void setUpdateInterval(const std::chrono::seconds &update_interval);
  std::chrono::seconds getUpdateInterval() const;
  bool begin();

  int getCO2Level() const { return sensed_co2_level; }

  void setUpdateCallback(const callback_t &cb);

  void startCalibration(int method);

private:
  std::chrono::seconds update_interval;
  MHZ19 sensor;
  std::thread *thread;
  std::atomic<int> sensed_co2_level;
  std::atomic<bool> exitflag;
  callback_t callback;

  static void thread_func(CO2Sensor *self);
};

#endif /* _CO2_SENSOR_H_ */
