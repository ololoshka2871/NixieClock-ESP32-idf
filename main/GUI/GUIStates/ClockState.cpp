#include "InterfaceButton.h"
#include "Nixie.h"
#include "RTC.h"

#include "GUI/Animations/ProgressLedAnimation.h"

#include "ClockState.h"

ClockState::ClockState(RTCManager *rtc)
    : AbstractGUIState(), rtc(rtc), color{std::make_unique<CRGB>()} {}

void ClockState::enter(ctl::InterfaceButton *btn, Nixie *indicators,
                       CFastLED *leds) {
  if (rtc) {
    rtc->setCallback(
        std::bind(&Nixie::setValue, indicators, std::placeholders::_1));
  }

  AbstractGUIState::enter(btn, indicators, leds);
  /*
  btn->onPush(std::bind(&ClockState::startLongPushProgress, this,
                        std::placeholders::_1, std::placeholders::_2, btn,
                        leds));
                        */
  setup_color();
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

void ClockState::setColor(const CRGB &newcolor) {
  *color = newcolor;
  setup_color();
}

const CRGB &ClockState::getColor() const { return *color; }

void ClockState::setup_color() {
  if (leds) {
    leds->showColor(*color);
  } else {
    ESP_LOGI(LOG_TAG, "Color setup skipped");
  }
}

void ClockState::startLongPushProgress(ctl::InterfaceButton::eventID id,
                                       gpio_num_t pin,
                                       ctl::InterfaceButton *btn,
                                       CFastLED *leds) {
  (void)id;
  (void)pin;

  LongPushAnimation =
      new ProgressLedAnimation{CRGB::White, *leds, 6, btn->longPushTime()};

  LongPushAnimation->play();

  btn->onRelease([this, btn](ctl::InterfaceButton::eventID id, gpio_num_t pin) {
    LongPushAnimation->stop();
    indicators->clear();
    btn->onRelease(nullptr);
  });
}
