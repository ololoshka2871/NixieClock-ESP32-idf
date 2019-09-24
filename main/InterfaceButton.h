#ifndef _INTERFACE_BUTTON_H_
#define _INTERFACE_BUTTON_H_

#include <cstdint>
#include <functional>
#include <string>

#include <driver/gpio.h>
#include <esp_timer.h>

struct InterfaceButton {

  enum eventID { Push = 0, Release, Click, LongPush, SIZE };

  using event_cb_t = std::function<void(eventID id, gpio_num_t pin)>;

  static constexpr uint64_t button_Check_period_us = 10000;
  static constexpr uint long_push_period =
      3 * 1000000 / button_Check_period_us; // 3s

  InterfaceButton(gpio_num_t gpio, bool active_level = false);
  ~InterfaceButton();

  void begin();

  void dispatchEvent(eventID id);

  InterfaceButton &onPush(event_cb_t &&cb);
  InterfaceButton &onRelease(event_cb_t &&cb);
  InterfaceButton &onClick(event_cb_t &&cb);
  InterfaceButton &onLongPush(event_cb_t &&cb);
  InterfaceButton &resetCallbacks();

private:
  static uint registredButtons;

  gpio_num_t pin;
  std::string eventGroupName;
  event_cb_t callbacks[SIZE];
  esp_timer_handle_t timerHandle;
  uint active_counter;
  bool active_level;

  bool getButtonState() const;
  void CheckButton();
};

#endif /*_INTERFACE_BUTTON_H_*/
