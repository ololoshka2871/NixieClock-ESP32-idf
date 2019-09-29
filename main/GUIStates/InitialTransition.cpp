#include <thread>

#include "FastLED.h"
#include "Nixie.h"

#include "InitialTransition.h"

InitialTransition::InitialTransition(AbstractGUIState *targetState)
    : AbstractGUIStateTransition{targetState} {}

void InitialTransition::Transit(Nixie *indicators, CFastLED *leds) {
  using namespace std::literals::chrono_literals;

  leds->clear(true);
  indicators->setValue("333333");
  std::this_thread::sleep_for(0.25s);
  leds->showColor(CRGB::Red);
  std::this_thread::sleep_for(0.25s);
  indicators->setValue("222222");
  std::this_thread::sleep_for(0.25s);
  leds->showColor(CRGB::Green);
  std::this_thread::sleep_for(0.25s);
  indicators->setValue("111111");
  std::this_thread::sleep_for(0.25s);
  leds->showColor(CRGB::Blue);
  std::this_thread::sleep_for(0.25s);
  indicators->setValue(Nixie::clear_indicators);

  AbstractGUIStateTransition::Transit(indicators, leds);
}
