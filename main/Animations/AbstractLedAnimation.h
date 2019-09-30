#ifndef _ABSTRACT_LED_ANIMATION_H_
#define _ABSTRACT_LED_ANIMATION_H_

#include <chrono>
#include <mutex>

#include "Timer.h"

class CFastLED;
class CRGB;

struct AbstractLedAnimation {
  static constexpr long FRAME_TIME_US = 10000;

  AbstractLedAnimation(CFastLED &leds, size_t ledCount,
                       const std::chrono::seconds &duration);

  virtual AbstractLedAnimation &play(bool cyclical = false);
  virtual AbstractLedAnimation &stop();
  virtual AbstractLedAnimation &reset();

  virtual AbstractLedAnimation &waitFinish();

private:
  CFastLED &leds;
  size_t ledCount;
  esp::Timer animationTimer;
  long animationFrame;
  const long animation_duration;
  bool cyclical;
  std::mutex animationInProgressMutex;

  void doAnimateFrame();

protected:
  const long framesPreLed;

  virtual void showFrame(long frame_number, CFastLED &leds,
                         size_t ledCount) = 0;

  CRGB &getLed(int n);
  CRGB &getLedRevesed(int n);
};

#endif /* _ABSTRACT_LED_ANIMATION_H_ */
