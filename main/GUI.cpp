#include <string>

#include <esp_log.h>

#include "InterfaceButton.h"

#include "GUIStates/CO2Monitor.h"
#include "GUIStates/ClockState.h"
#include "GUIStates/DateDisplayer.h"
#include "GUIStates/TemperatureMonitor.h"
#include "GUIStates/WifiEnabler.h"

#include "GUI.h"

static ClockState clockState;
static CO2Monitor co2Monitor;
static TemperatureMonitor temperatureMonitor;
static DateDisplay dateDisplay;
static WifiEnabler wifiEnabler;

static InterfaceButton btn{GPIO_NUM_0};

struct Logger {
  static constexpr char LOG_TAG[] = "Btn event";

  Logger(const std::string &msg) : msg(msg) {}

  void operator()(InterfaceButton::eventID id, gpio_num_t pin) {
    ESP_LOGW(LOG_TAG, "%s", msg.c_str());
  }

private:
  const std::string msg;
};

void GUI::init() {
  btn.begin();
  btn.onPush(Logger("Pushed"))
      .onRelease(Logger("Released"))
      .onClick(Logger("Clicked"))
      .onLongPush(Logger("Long pushed"));
}
