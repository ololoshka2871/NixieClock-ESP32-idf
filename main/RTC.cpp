#include <ctime>

#include <driver/gpio.h>
#include <esp_log.h>

#include "Controller.h"
#include "String_format.h"

#include "RTC.h"

static constexpr gpio_num_t SQW_PIN = GPIO_NUM_23;
static constexpr gpio_num_t SDA_PIN = GPIO_NUM_21;
static constexpr gpio_num_t SCL_PIN = GPIO_NUM_22;

static constexpr uint32_t Sync_Clock_EVERY_s = 60;

void RTCManager::register_rtc_interrupt() {
#ifndef TEST_MODE
  gpio_pad_select_gpio(SQW_PIN);
  gpio_set_direction(SQW_PIN, GPIO_MODE_INPUT);
  gpio_set_pull_mode(SQW_PIN, GPIO_FLOATING);
  gpio_set_intr_type(SQW_PIN, GPIO_INTR_NEGEDGE);
  gpio_intr_enable(SQW_PIN);

  gpio_isr_handler_add(
      SQW_PIN,
      [](void *ctx) {
        xSemaphoreGiveFromISR(static_cast<SemaphoreHandle_t>(ctx), nullptr);
      },
      rtc_sem);
#endif
}

void RTCManager::load_clock() {
  tm time{};

  if (ds1307_get_time(&ds1307_dev, &time) != ESP_OK) {
    ESP_LOGE(LOG_TAG, "Failed to read RTC! Skipping clock ajustment");
    return;
  }
  fix_tm(time);

  const struct timeval tv { std::mktime(&time), 0 };
  settimeofday(&tv, nullptr);

  ESP_LOGI(LOG_TAG, "RTC Clock syncronised: %s", std::asctime(&time));
}

void RTCManager::thread_func(RTCManager *self) {
  self->register_rtc_interrupt();
  self->enable_1s_interrupt();
  while (true) {
    if (self->exitflag)
      return;
    if (xSemaphoreTake(self->rtc_sem, portMAX_DELAY) == pdTRUE) {
      std::time_t result{std::time(nullptr)};
      std::tm tm{*std::gmtime(&result)};
      std::string s = format("%02d%02d%02d", tm.tm_hour, tm.tm_min, tm.tm_sec);

      if (self->cb) {
        self->cb(s);
      }

      ++self->sync_counter;
      if (self->sync_counter == Sync_Clock_EVERY_s) {
        self->load_clock();
        self->sync_counter = 0;
      }
    }
  }
}

RTCManager::RTCManager(uint8_t rtc_addr)
    : rtc_sem(xSemaphoreCreateBinary()), exitflag(false), cb{}, ds1307_dev{},
      sync_counter(0) {
  ESP_ERROR_CHECK(ds1307_init_desc(&ds1307_dev, I2C_NUM_0, SDA_PIN, SCL_PIN));
  ds1307_dev.addr = rtc_addr;
}

RTCManager::~RTCManager() {
  exitflag = true;
  update_thread->join();
  i2c_driver_delete(I2C_NUM_0);
}

RTCManager &RTCManager::loadTime() {
  load_clock();
  return *this;
}

RTCManager &RTCManager::setupRTC(const std::time_t &dest_time) {
  std::tm tm;
  if (dest_time == 0) {
    std::time_t result{std::time(nullptr)};
    tm = std::tm{*std::gmtime(&result)};
  } else {
    tm = std::tm{*std::gmtime(&dest_time)};
  }
  unfix_tm(tm);

  if (ds1307_set_time(&ds1307_dev, &tm) != ESP_OK) {
    ESP_LOGE(LOG_TAG, "Failed to set RTC!");
  }

  return *this;
}

void RTCManager::enable_1s_interrupt() {
  ds1307_set_squarewave_freq(&ds1307_dev, DS1307_1HZ);
  ds1307_enable_squarewave(&ds1307_dev, true);
}

RTCManager &RTCManager::begin() {
  update_thread = new std::thread{&RTCManager::thread_func, this};
  return *this;
}

RTCManager &
RTCManager::setCallback(const RTCManager::onTimeUpdated &onTimeUpdated) {
  cb = onTimeUpdated;
  return *this;
}
