#ifndef TEMPERATURE_SENSOR_H_
#define TEMPERATURE_SENSOR_H_

#include <atomic>
#include <chrono>
#include <thread>

#include "ds18x20.h"

template <typename Rep, typename Period> struct TemperatureSensor {
  static constexpr char LOG_TAG[] = "TemperatureSensor";

  TemperatureSensor(gpio_num_t OneWirePin,
                    const std::chrono::duration<Rep, Period> &updatePeriod)
      : OneWirePin(OneWirePin), temperature_update_interval(updatePeriod),
        sensed_temperature(20.0f), exitflag(false) {}
  TemperatureSensor(const TemperatureSensor &) = delete;
  ~TemperatureSensor() {
    exitflag = true;
    temperature_reader->join();
    delete temperature_reader;
  }

  bool begin() {
    auto sensor_count = ds18x20_scan_devices(OneWirePin, &sensorAddress, 1);
    if (sensor_count != 1) {
      ESP_LOGE(LOG_TAG,
               "No ds18x20 devices found, temperatire sensor will not work!");
      return false;
    }
    ESP_LOGI(LOG_TAG, "ds18x20 sendsor found at address 0x%llX", sensorAddress);
    temperature_reader =
        new std::thread(std::thread{&TemperatureSensor::thread_func, this});
    return temperature_reader != nullptr;
  }

  float temperature() const { return sensed_temperature; }

private:
  const gpio_num_t OneWirePin;
  const std::chrono::duration<Rep, Period> temperature_update_interval;
  std::thread *temperature_reader;
  ds18x20_addr_t sensorAddress;
  std::atomic<float> sensed_temperature;
  std::atomic<bool> exitflag;

  static void thread_func(TemperatureSensor *self) {
    float T;
    while (true) {
      if (self->exitflag)
        return;

      auto err =
          ds18x20_measure_and_read(self->OneWirePin, self->sensorAddress, &T);
      if (err == ESP_OK) {
        ESP_LOGI(LOG_TAG, "Temperatire: %f *C", T);
        self->sensed_temperature = T;
      } else {
        ESP_LOGE(LOG_TAG, "Failed to read temperature from ds18x20, code: %d",
                 err);
      }

      std::this_thread::sleep_for(self->temperature_update_interval);
    }
  }
};

#endif /* TEMPERATURE_SENSOR_H_ */
