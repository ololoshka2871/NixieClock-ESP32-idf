#include <cassert>
#include <functional>

#include "FastLED.h"

#include "AbstractLedAnimation.h"

AbstractLedAnimation::AbstractLedAnimation(CFastLED &leds, size_t ledCount,
                                           const std::chrono::seconds &duration)
    : leds(leds),
      ledCount(ledCount), animationTimer{std::bind(
                              &AbstractLedAnimation::doAnimateFrame, this)},
      animationFrame{},
      animation_duration(
          std::chrono::duration_cast<std::chrono::microseconds>(duration)
              .count() /
          FRAME_TIME_US),
      cyclical{false}, framesPreLed{animation_duration / (long)ledCount} {}

AbstractLedAnimation &AbstractLedAnimation::play(bool cyclical) {
  reset();
  this->cyclical = cyclical;
  animationTimer.start(FRAME_TIME_US, true);
  animationInProgressMutex.lock();
  return *this;
}

AbstractLedAnimation &AbstractLedAnimation::stop() {
  animationTimer.stop();
  animationInProgressMutex.unlock();
  return *this;
}

AbstractLedAnimation &AbstractLedAnimation::reset() {
  animationFrame = 0;
  return *this;
}

AbstractLedAnimation &AbstractLedAnimation::waitFinish() {
  std::lock_guard guard(animationInProgressMutex);
  return *this;
}

CRGB &AbstractLedAnimation::getLed(int n) {
  assert(n >= 0 && n < ledCount);
  return leds.leds()[n];
}

CRGB &AbstractLedAnimation::getLedRevesed(int n) {
  assert(n >= 0 && n < ledCount);
  return leds.leds()[ledCount - 1 - n];
}

void AbstractLedAnimation::doAnimateFrame() {
  ++animationFrame;
  showFrame(animationFrame, leds, ledCount);
  leds.show();
  if (animationFrame == animation_duration) {
    if (cyclical) {
      reset();
    } else {
      stop();
    }
  }
}
