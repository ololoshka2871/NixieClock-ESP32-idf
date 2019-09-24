#ifndef _IGUI_STATE_H_
#define _IGUI_STATE_H_

#include "IGUIStateTransition.h"

struct IGUIState {
  IGUIStateTransition *clickTransition;
  IGUIStateTransition *LongPushTransition;
  IGUIStateTransition *IdleTransition;
};

#endif /* _IGUI_STATE_H_ */
