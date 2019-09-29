#ifndef _ABSTRACT_GUI_STATE_TRANSITION_
#define _ABSTRACT_GUI_STATE_TRANSITION_

#include <esp_log.h>

#include "GUI.h"

struct AbstractGUIState;
struct Nixie;
class CFastLED;

struct AbstractGUIStateTransition {
  AbstractGUIStateTransition(AbstractGUIState *targetState)
      : targetState(targetState) {}

  virtual void Transit(Nixie *indicators, CFastLED *leds) {
    ESP_LOGI(getLOG_TAG(), "Transit()");
    GUI::setCurrentState(targetState);
  }

  virtual const char *getLOG_TAG() const { return LOG_TAG; }

private:
  static constexpr char LOG_TAG[] = "AbstractGUIStateTransition";

  AbstractGUIState *targetState;
};

#endif /* _ABSTRACT_GUI_STATE_TRANSITION_ */
