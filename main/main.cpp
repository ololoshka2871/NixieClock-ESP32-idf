#include <ctime>

#include <driver/gpio.h>
#include <esp_log.h>

#include "i2cdev.h"

#include "Nixie.h"
#include "RTC.h"
#include "TemperatureSensor.h"
#include "VSensors.h"

#include "FastLED.h"
#include "MHZ19.h"

static RTCManager rtc_ctrl;
static TemperatureSensor ds18b20{GPIO_NUM_4, std::chrono::seconds(10)};
static CRGB leds[6];
static MHZ19 mhz_19{UART_NUM_2, GPIO_NUM_16, GPIO_NUM_17};
static VSensors voltage_sensors{
    VSensChanel{ADC1_CHANNEL_3, 4.7f, 1.0f},  // 3.3v
    VSensChanel{ADC1_CHANNEL_7, 9.1f, 1.0f},  // 5 v
    VSensChanel{ADC1_CHANNEL_6, 24.0f, 1.0f}, // 12 v
};

static constexpr char LOG_TAG[] = "app_main";

extern "C" void app_main(void) {
  gpio_install_isr_service(0); // interrupt for all gpio events
  ESP_ERROR_CHECK(i2cdev_init());

  Nixie::configure();

#ifndef TEST_MODE
  rtc_ctrl.loadTime();
  rtc_ctrl.setCallback(std::bind(Nixie::setValue, std::placeholders::_1))
      .begin();
#endif

  ds18b20.begin();

  // turn off GPIO logging
  esp_log_level_set("gpio", ESP_LOG_NONE);

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

  ESP_LOGI("MHZ-19", "CO2 = %d [ppm], T = %d [*C]", mhz_19.getPPM(),
           mhz_19.getTemperature());

  for (size_t i = 0; i < 3; ++i) {
    auto val = voltage_sensors.getChannelVoltage(i);
    ESP_LOGI("Voltage", "Channel %d voltage: %f V", i, val);
  }
}
