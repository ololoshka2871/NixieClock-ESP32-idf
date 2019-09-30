#include <cassert>

#include "Nixie.h"

#include "Animations/ProgressLedAnimation.h"

#include "ProgressTransition.h"

ProgressTransition::ProgressTransition(
    AbstractGUIState *targetState,
    const std::chrono::seconds &animation_duration_s, const CRGB &color)
    : AbstractGUIStateTransition{targetState}, baseColor(color),
      animation_duration_s(animation_duration_s) {
  assert(animation_duration_s.count() > 0);
}

void ProgressTransition::Transit(Nixie *indicators, CFastLED *leds) {
  indicators->clear();
  ProgressLedAnimation animation{baseColor, *leds, led_count,
                                 animation_duration_s};
  animation.play().waitFinish();
  AbstractGUIStateTransition::Transit(indicators, leds);
}
