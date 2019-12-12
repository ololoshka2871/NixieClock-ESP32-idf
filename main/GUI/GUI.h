#ifndef _GUI_H_
#define _GUI_H_

#include "InterfaceButton.h"

struct AbstractGUIState;
struct RTCManager;
struct CO2Sensor;
struct TemperatureSensor;

struct GUI {
  static constexpr char LOG_TAG[] = "GUI";

  static constexpr char clock_color[] = "clock_color";

  static void init(CO2Sensor *CO2Sensor, TemperatureSensor *TSensor);
  static void start();

  static uint8_t getClockBGColorComponenta(const uint8_t n);
  static void setClockBGColorComponenta(const uint8_t n, uint8_t _v);

  static uint32_t getClockBGColor();
  static void setClockBGColor(uint32_t newcolor);

  static void setCurrentState(AbstractGUIState *newstate);

private:
  static AbstractGUIState *currentState;

  static void setClockBGColor_no_save(uint32_t newcolor);
};

#endif /* _GUI_H_ */
