#include <esp_log.h>

#include "TemperatureSensor.h"

TemperatureSensor::TemperatureSensor(gpio_num_t OneWirePin,
                                     const std::chrono::seconds &updatePeriod_s)
    : OneWirePin(OneWirePin), temperature_update_interval(updatePeriod_s),
      sensed_temperature(20.0f), exitflag(false) {}

TemperatureSensor::~TemperatureSensor() {
  exitflag = true;
  temperature_reader->join();
  delete temperature_reader;
}

void TemperatureSensor::setUpdateInterval(
    const std::chrono::seconds &update_interval) {
  this->temperature_update_interval = update_interval;
}

std::chrono::seconds TemperatureSensor::getUpdateInterval() const {
  return temperature_update_interval;
}

bool TemperatureSensor::begin() {
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

float TemperatureSensor::temperature() const { return sensed_temperature; }

void TemperatureSensor::setUpdateCallback(
    const TemperatureSensor::callback_t &cb) {
  callback = cb;
}

void TemperatureSensor::thread_func(TemperatureSensor *self) {
  using tick_duration_t =
      std::chrono::duration<int, std::ratio<1, CONFIG_FREERTOS_HZ>>;

  float T;
  while (true) {
    auto err =
        ds18x20_measure_and_read(self->OneWirePin, self->sensorAddress, &T);
    if (err == ESP_OK) {
      ESP_LOGI(LOG_TAG, "Temperatire: %f *C", T);
      self->sensed_temperature = T;
      if (self->callback) {
        self->callback(T);
      }
    } else {
      ESP_LOGE(LOG_TAG, "Failed to read temperature from ds18x20, code: %d",
               err);
    }

    for (size_t i = 0; i < std::chrono::duration_cast<tick_duration_t>(
                               self->temperature_update_interval)
                               .count();
         ++i) {
      if (self->exitflag)
        return;
      std::this_thread::sleep_for(tick_duration_t(1));
    }
  }
}
