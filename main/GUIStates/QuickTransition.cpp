#include "FastLED.h"
#include "Nixie.h"

#include "QuickTransition.h"

void QuickTransition::Transit(Nixie *indicators, CFastLED *leds) {
  indicators->clear();
  leds->clear(true);

  AbstractGUIStateTransition::Transit(indicators, leds);
}
