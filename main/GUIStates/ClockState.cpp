#include "InterfaceButton.h"
#include "Nixie.h"
#include "RTC.h"

#include "Animations/ProgressLedAnimation.h"

#include "ClockState.h"

void ClockState::enter(InterfaceButton *btn, Nixie *indicators,
                       CFastLED *leds) {
  if (rtc) {
    rtc->setCallback(
        std::bind(&Nixie::setValue, indicators, std::placeholders::_1));
  }

  AbstractGUIState::enter(btn, indicators, leds);
  btn->onPush(std::bind(&ClockState::startLongPushProgress, this,
                        std::placeholders::_1, std::placeholders::_2, btn,
                        leds));

  btn->dumpCallbacks();
}

void ClockState::leave() {
  if (rtc) {
    rtc->setCallback(nullptr);
  }
  if (LongPushAnimation) {
    LongPushAnimation->stop();
    delete LongPushAnimation;
  }

  AbstractGUIState::leave();
}

void ClockState::startLongPushProgress(InterfaceButton::eventID id,
                                       gpio_num_t pin, InterfaceButton *btn,
                                       CFastLED *leds) {
  (void)id;
  (void)pin;

  LongPushAnimation =
      new ProgressLedAnimation{CRGB::White, *leds, 6, btn->longPushTime()};

  LongPushAnimation->play();

  btn->onRelease([this, btn](InterfaceButton::eventID id, gpio_num_t pin) {
    LongPushAnimation->stop();
    indicators->clear();
    btn->onRelease(nullptr);
  });
}
