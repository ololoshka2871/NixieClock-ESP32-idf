#ifndef _GUI_H_
#define _GUI_H_

#include "InterfaceButton.h"

class AbstractGUIState;

struct GUI {
  static constexpr char LOG_TAG[] = "GUI";

  static void init();
  static void start();

  static void setCurrentState(AbstractGUIState *newstate);

private:
  static AbstractGUIState *currentState;
};

#endif /* _GUI_H_ */
