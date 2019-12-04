#include <esp_log.h>

#include "CO2Sensor.h"

CO2Sensor::CO2Sensor(const uart_port_t uart_num, const gpio_num_t rx,
                     const gpio_num_t tx,
                     const std::chrono::seconds &update_interval)
    : update_interval(update_interval), sensor{uart_num, rx, tx} {}

void CO2Sensor::setUpdateInterval(const std::chrono::seconds &update_interval) {
  this->update_interval = update_interval;
}

std::chrono::seconds CO2Sensor::getUpdateInterval() const {
  return update_interval;
}

bool CO2Sensor::begin() {
  thread = new std::thread{thread_func, this};
  return thread != nullptr;
}

CO2Sensor::~CO2Sensor() {
  exitflag = true;
  thread->join();
  delete thread;
}

void CO2Sensor::setUpdateCallback(const callback_t &cb) { callback = cb; }

void CO2Sensor::thread_func(CO2Sensor *self) {
  using tick_duration_t =
      std::chrono::duration<int, std::ratio<1, CONFIG_FREERTOS_HZ>>;

  while (true) {
    auto co2_level = self->sensor.getPPM();

    ESP_LOGI(LOG_TAG, "CO2 level: %d ppm", co2_level);
    self->sensed_co2_level = co2_level;

    if (self->callback) {
      self->callback(co2_level);
    }

    for (size_t i = 0;
         i < std::chrono::duration_cast<tick_duration_t>(self->update_interval)
                 .count();
         ++i) {
      if (self->exitflag)
        return;
      std::this_thread::sleep_for(tick_duration_t(1));
    }
  }
}
