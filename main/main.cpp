#include <ctime>

#include <driver/gpio.h>
#include <esp_log.h>

#include "i2cdev.h"

#include "Nixie.h"
#include "RTC.h"
#include "TemperatureSensor.h"

#include "FastLED.h"

static RTCManager rtc_ctrl;
static TemperatureSensor ds18b20{GPIO_NUM_4, std::chrono::seconds(10)};
CRGB leds[6];

static constexpr char LOG_TAG[] = "app_main";

static void set_RTC_to_build_time() {
  tm build_time{CLOCK_START_SEC, CLOCK_START_MIN,     CLOCK_START_HOUR,
                CLOCK_START_DAY, CLOCK_START_MON - 1, CLOCK_START_YEAR};
  RTCManager::fix_tm(build_time);

  const struct timeval tv { std::mktime(&build_time), 0 };
  settimeofday(&tv, nullptr);

  ESP_LOGI(LOG_TAG, "RTC Clock set to build time: %s",
           std::asctime(&build_time));
}

extern "C" void app_main(void) {
  set_RTC_to_build_time();

  gpio_install_isr_service(0); // interrupt for all gpio events
  ESP_ERROR_CHECK(i2cdev_init());

  Nixie::configure();

  rtc_ctrl
#ifndef TEST_MODE
      //.setupRTC()
      .loadTime()
#endif
      .setCallback(std::bind(Nixie::setValue, std::placeholders::_1))
      .begin();

  ds18b20.begin();

  FastLED.addLeds<NEOPIXEL,
#ifdef TEST_MODE
                  GPIO_NUM_4
#else
                  GPIO_NUM_15
#endif
                  >(leds, 6);

  for (auto &led : leds) {
    led = CRGB::Red;
  }
  FastLED.show();
}
