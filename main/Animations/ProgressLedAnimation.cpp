#include "ProgressLedAnimation.h"

ProgressLedAnimation::ProgressLedAnimation(const CRGB &baseColor,
                                           CFastLED &leds, size_t ledCount,
                                           const std::chrono::seconds &duration)
    : AbstractLedAnimation(leds, ledCount, duration), baseColor(baseColor) {}

void ProgressLedAnimation::showFrame(long frame_number, CFastLED &leds,
                                     size_t ledCount) {
  const int filledCount = frame_number / framesPreLed;
  const float ov = frame_number % framesPreLed;

  CRGB addColor1{baseColor};
  addColor1.fadeLightBy(
      static_cast<uint8_t>(std::floor(255 * (1.0f - ov / framesPreLed))));
  CRGB addColor2{baseColor};
  addColor2.fadeLightBy(
      static_cast<uint8_t>(std::floor(220 * (ov / framesPreLed))));

  int i = filledCount;
  if (i > 0) {
    getLedRevesed(i - 1) = addColor2;
  }
  if (i < ledCount) {
    getLedRevesed(i) = addColor1;
  }
}
