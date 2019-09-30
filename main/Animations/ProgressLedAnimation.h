#ifndef _PROGRESS_LED_ANIMATION_H_
#define _PROGRESS_LED_ANIMATION_H_

#include "FastLED.h"

#include "AbstractLedAnimation.h"

struct ProgressLedAnimation : public AbstractLedAnimation {
  ProgressLedAnimation(const CRGB &baseColor, CFastLED &leds, size_t ledCount,
                       const std::chrono::seconds &duration);

protected:
  void showFrame(long frame_number, CFastLED &leds, size_t ledCount) override;

private:
  CRGB baseColor;
};

#endif /* _PROGRESS_LED_ANIMATION_H_ */
