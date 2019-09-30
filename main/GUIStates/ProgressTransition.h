#ifndef _PROGRESS_TRANSITION_H_
#define _PROGRESS_TRANSITION_H_

#include <chrono>

#include "FastLED.h"

#include "Timer.h"

#include "AbstractGUIStateTransition.h"

class CFastLED;
class CRGB;

struct ProgressTransition : public AbstractGUIStateTransition {
  static constexpr uint64_t FRAME_TIME_US = 10000;
  static constexpr int led_count = 6;

  ProgressTransition(AbstractGUIState *targetState,
                     std::chrono::seconds animation_duration_s,
                     const CRGB &baseColor = CRGB::White);

  const char *getLOG_TAG() const { return LOG_TAG; }

  void Transit(Nixie *indicators, CFastLED *leds) override;

private:
  static constexpr char LOG_TAG[] = "ProgressTransition";

  size_t animationFrame;
  const size_t animation_duration;
  esp::Timer frameTimer;
  Nixie *indicators;
  CFastLED *leds;
  const CRGB baseColor;
  const uint framesPreLed;

  void resetAnimation();
  void animation_frame();
  void drawFrame();
  CRGB &led(int i);
};

#endif /* _PROGRESS_TRANSITION_H_ */
