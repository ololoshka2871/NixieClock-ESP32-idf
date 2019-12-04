#ifndef _CLOCK_STATE_H_
#define _CLOCK_STATE_H_

#include "AbstractGUIState.h"

struct RTCManager;
struct AbstractLedAnimation;

struct ClockState : public AbstractGUIState {
  ClockState(RTCManager *rtc = nullptr) : AbstractGUIState(), rtc(rtc) {}

  void enter(ctl::InterfaceButton *btn, Nixie *indicators,
             CFastLED *leds) override;
  void leave() override;

  uint64_t idleTimeout() const override { return 0; }
  const char *getLOG_TAG() const override { return LOG_TAG; }

  void setRTC(RTCManager *rtc) { this->rtc = rtc; }

private:
  static constexpr char LOG_TAG[] = "ClockState";

  RTCManager *rtc;
  AbstractLedAnimation *LongPushAnimation;

  void clock_tick();
  void startLongPushProgress(ctl::InterfaceButton::eventID id, gpio_num_t pin,
                             ctl::InterfaceButton *btn, CFastLED *leds);
};

#endif /* _CLOCK_STATE_H_ */
