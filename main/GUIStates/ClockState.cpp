
#include "Nixie.h"
#include "RTC.h"

#include "ClockState.h"

void ClockState::enter(InterfaceButton *btn, Nixie *indicators,
                       CFastLED *leds) {
  if (rtc) {
    rtc->setCallback(
        std::bind(&Nixie::setValue, indicators, std::placeholders::_1));
  }

  AbstractGUIState::enter(btn, indicators, leds);
}

void ClockState::leave() {
  if (rtc) {
    rtc->setCallback(nullptr);
  }
  AbstractGUIState::leave();
}
