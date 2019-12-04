#ifndef _PROGRESS_TRANSITION_H_
#define _PROGRESS_TRANSITION_H_

#include <chrono>

#include "FastLED.h"

#include "AbstractGUIStateTransition.h"

struct ProgressTransition : public AbstractGUIStateTransition {
  static constexpr int led_count = 6;

  ProgressTransition(AbstractGUIState *targetState,
                     const std::chrono::seconds &animation_duration_s,
                     const CRGB &baseColor = CRGB::White);

  const char *getLOG_TAG() const { return LOG_TAG; }

  void Transit(Nixie *indicators, CFastLED *leds) override;

private:
  static constexpr char LOG_TAG[] = "ProgressTransition";

  const CRGB baseColor;
  const std::chrono::seconds animation_duration_s;
};

#endif /* _PROGRESS_TRANSITION_H_ */
