#include "FastLED.h"
#include "Nixie.h"

#include "QuickTransition.h"

void QuickTransition::Transit(Nixie *indicators, CFastLED *leds) {
  indicators->setValue(Nixie::clear_indicators);
  leds->clear(true);

  AbstractGUIStateTransition::Transit(indicators, leds);
}
