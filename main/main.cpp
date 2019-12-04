#include <ctime>
#include <sstream>
#include <vector>

#include <driver/gpio.h>
#include <esp_log.h>

#include "i2cdev.h"

//#include "HttpServer.h"
#include "CO2Sensor.h"
#include "GUI.h"
#include "Nixie.h"
#include "RTC.h"
#include "TemperatureSensor.h"
#include "VSensors.h"
#include "VirtuinoJsonServer.h"
#include "monitor.h"

#include "FastLED.h"

#ifndef TEST_MODE
static RTCManager rtc_ctrl;
static TemperatureSensor ds18b20{GPIO_NUM_4, std::chrono::minutes(1)};
static CRGB leds[6];
static CO2Sensor mhz_19{UART_NUM_2, GPIO_NUM_16, GPIO_NUM_17};
static VSensors voltage_sensors{
    VSensChanel{ADC1_CHANNEL_3, 4.7f, 1.0f},  // 3.3v
    VSensChanel{ADC1_CHANNEL_7, 9.1f, 1.0f},  // 5 v
    VSensChanel{ADC1_CHANNEL_6, 24.0f, 1.0f}, // 12 v
};
#endif

static constexpr char LOG_TAG[] = "app_main";

using svect_t = std::vector<std::string>;

extern "C" void app_main(void) {
  gpio_install_isr_service(0); // interrupt for all gpio events
  ESP_ERROR_CHECK(i2cdev_init());

#if 1
  auto monitor = new app::Monitor();
#endif

#ifndef TEST_MODE
  rtc_ctrl.loadTime().begin();
#endif

#ifndef TEST_MODE
  ds18b20.begin();
  mhz_19.begin();
#endif

  // turn off GPIO logging
  esp_log_level_set("gpio", ESP_LOG_NONE);

#ifndef TEST_MODE
  FastLED.addLeds<NEOPIXEL, GPIO_NUM_15>(leds, 6);
#endif

#ifndef TEST_MODE
  /*
  ESP_LOGI("MHZ-19", "CO2 = %d [ppm], T = %d [*C]", mhz_19.getPPM(),
           mhz_19.getTemperature());
           */

  for (size_t i = 0; i < 3; ++i) {
    auto val = voltage_sensors.getChannelVoltage(i);
    ESP_LOGI("Voltage", "Channel %d voltage: %f V", i, val);
  }
#endif

#if 0
  HttpServer::begin();
  HttpServer::start();
#endif

#if 0
  VirtuinoJsonServer::begin();
  VirtuinoJsonServer::start();
#endif

  GUI::init(&rtc_ctrl, &mhz_19, &ds18b20);
  GUI::start();
}
