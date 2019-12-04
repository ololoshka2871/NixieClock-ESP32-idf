#ifndef _GUI_H_
#define _GUI_H_

#include "InterfaceButton.h"

struct AbstractGUIState;
struct RTCManager;
struct CO2Sensor;
struct TemperatureSensor;

struct GUI {
  static constexpr char LOG_TAG[] = "GUI";

  static void init(RTCManager *rtc, CO2Sensor *CO2Sensor,
                   TemperatureSensor *TSensor);
  static void start();

  static void setCurrentState(AbstractGUIState *newstate);

private:
  static AbstractGUIState *currentState;
};

#endif /* _GUI_H_ */
