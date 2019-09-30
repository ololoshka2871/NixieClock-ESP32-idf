#ifndef _INTERFACE_BUTTON_H_
#define _INTERFACE_BUTTON_H_

#include <chrono>
#include <cstdint>
#include <functional>
#include <string>

#include <driver/gpio.h>
#include <esp_timer.h>

struct InterfaceButton {

  enum eventID { Push = 0, Release, Click, LongPush, SIZE };

  using event_cb_t = std::function<void(eventID id, gpio_num_t pin)>;

  using check_period_t =
      std::chrono::duration<uint64_t, std::ratio<10000, 1000000>>; // 10ms

  static constexpr char LOG_TAG[] = "InterfaceButton";

  inline static const std::chrono::microseconds buttonCheckPeriod =
      std::chrono::duration_cast<std::chrono::microseconds>(check_period_t(1));
  inline static const check_period_t longPushPeriod =
      std::chrono::duration_cast<check_period_t>(std::chrono::seconds(3));

  InterfaceButton(gpio_num_t gpio, bool active_level = false);
  ~InterfaceButton();

  void begin();

  void dispatchEvent(eventID id);

  InterfaceButton &onPush(event_cb_t &&cb);
  InterfaceButton &onRelease(event_cb_t &&cb);
  InterfaceButton &onClick(event_cb_t &&cb);
  InterfaceButton &onLongPush(event_cb_t &&cb);
  InterfaceButton &resetCallbacks();

  std::chrono::seconds longPushTime() const;

  void dumpCallbacks() const;

private:
  static uint registredButtons;

  gpio_num_t pin;
  std::string eventGroupName;
  event_cb_t callbacks[SIZE];
  esp_timer_handle_t timerHandle;
  check_period_t active_counter;
  bool active_level;

  bool getButtonState() const;
  void CheckButton();
};

#endif /*_INTERFACE_BUTTON_H_*/
