#include <string>

#include <esp_log.h>

#include "InterfaceButton.h"

#include "GUIStates/CO2Monitor.h"
#include "GUIStates/ClockState.h"
#include "GUIStates/DateDisplayer.h"
#include "GUIStates/TemperatureMonitor.h"
#include "GUIStates/WifiEnabler.h"

#include "GUIStates/InitialTransition.h"
#include "GUIStates/ProgressTransition.h"
#include "GUIStates/QuickTransition.h"

#include "FastLED.h"

#include "Nixie.h"
#include "RTC.h"

#include "GUI.h"

static ClockState clockState;
static CO2Monitor co2Monitor, co2MonitorPresistant(true);
static TemperatureMonitor temperatureMonitor,
    temperatureMonitorPresistant(true);
static DateDisplay dateDisplay;
static WifiEnabler wifiEnabler;

static InitialTransition initialTransition{&clockState};

static ctl::InterfaceButton btn{GPIO_NUM_0};

AbstractGUIState *GUI::currentState = nullptr;

struct Logger {
  static constexpr char LOG_TAG[] = "Btn event";

  Logger(const std::string &msg) : msg(msg) {}

  void operator()(ctl::InterfaceButton::eventID id, gpio_num_t pin) {
    ESP_LOGW(LOG_TAG, "%s", msg.c_str());
  }

private:
  const std::string msg;
};

static void setupSensors(RTCManager *rtc, CO2Sensor *CO2Sensor,
                         TemperatureSensor *TSensor) {
  clockState.setRTC(rtc);

  co2Monitor.setSensor(CO2Sensor);
  co2MonitorPresistant.setSensor(CO2Sensor);

  temperatureMonitor.setSensor(TSensor);
  temperatureMonitorPresistant.setSensor(TSensor);
}

void GUI::init(RTCManager *rtc, CO2Sensor *CO2Sensor,
               TemperatureSensor *TSensor) {
  using namespace std::literals::string_literals;

  btn.begin();
  btn.onPush(Logger("Pushed"s)).onRelease(Logger("Released"s));

  setupSensors(rtc, CO2Sensor, TSensor);

  auto quickReturnToClock = std::make_shared<QuickTransition>(&clockState);

  // idles -> return to clock
  co2Monitor.IdleTransition = temperatureMonitor.IdleTransition =
      dateDisplay.IdleTransition = wifiEnabler.IdleTransition =
          quickReturnToClock;

  // click -> return to clock
  wifiEnabler.clickTransition = dateDisplay.clickTransition =
      co2MonitorPresistant.clickTransition =
          temperatureMonitorPresistant.clickTransition = quickReturnToClock;

  // CO2 -> CO2 presistant
  co2Monitor.LongPushTransition =
      std::make_shared<QuickTransition>(&co2MonitorPresistant);

  // Temperature -> Temperature presistant
  temperatureMonitor.LongPushTransition =
      std::make_shared<QuickTransition>(&temperatureMonitorPresistant);

  // Clock -> wifi
  clockState.LongPushTransition =
      std::make_shared<QuickTransition>(&wifiEnabler);

  // clock -> CO2
  clockState.clickTransition = std::make_shared<QuickTransition>(&co2Monitor);

  // CO2 -> Temperature
  co2Monitor.clickTransition =
      std::make_shared<QuickTransition>(&temperatureMonitor);

  // Temperature -> Date
  temperatureMonitor.clickTransition =
      std::make_shared<QuickTransition>(&dateDisplay);
}

void GUI::start() { initialTransition.Transit(Nixie::instance(), &FastLED); }

void GUI::setCurrentState(AbstractGUIState *newstate) {
  currentState = newstate;
  currentState->enter(&btn, Nixie::instance(), &FastLED);
}
