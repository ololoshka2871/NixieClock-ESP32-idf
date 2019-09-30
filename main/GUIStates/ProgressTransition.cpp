#include <cassert>

#include "Nixie.h"

#include "ProgressTransition.h"

ProgressTransition::ProgressTransition(
    AbstractGUIState *targetState, std::chrono::seconds animation_duration_s,
    const CRGB &color)
    : AbstractGUIStateTransition{targetState},
      animation_duration(std::chrono::duration_cast<std::chrono::microseconds>(
                             animation_duration_s)
                             .count() /
                         FRAME_TIME_US),
      frameTimer{std::bind(&ProgressTransition::animation_frame, this)},
      baseColor(color), framesPreLed(animation_duration / led_count) {
  assert(animation_duration_s.count() > 0);
}

void ProgressTransition::Transit(Nixie *indicators, CFastLED *leds) {
  this->indicators = indicators;
  this->leds = leds;
  resetAnimation();
}

void ProgressTransition::resetAnimation() {
  indicators->clear();
  leds->clear();
  animationFrame = 0;

  frameTimer.start(FRAME_TIME_US, true);
}

void ProgressTransition::animation_frame() {
  ++animationFrame;
  drawFrame();
  if (animationFrame == animation_duration) {
    frameTimer.stop();
    AbstractGUIStateTransition::Transit(indicators, leds);
  }
}

// +++*^_
void ProgressTransition::drawFrame() {
  const int filledCount = animationFrame / framesPreLed;
  const float ov = animationFrame % framesPreLed;

  CRGB addColor1{baseColor};
  addColor1.fadeLightBy(
      static_cast<uint8_t>(std::floor(255 * (1.0f - ov / framesPreLed))));
  CRGB addColor2{baseColor};
  addColor2.fadeLightBy(
      static_cast<uint8_t>(std::floor(220 * (ov / framesPreLed))));

  int i = filledCount;
  if (i > 0) {
    led(i - 1) = addColor2;
  }
  if (i < led_count) {
    led(i) = addColor1;
  }

  leds->show();
}

CRGB &ProgressTransition::led(int i) { return leds->leds()[led_count - 1 - i]; }
