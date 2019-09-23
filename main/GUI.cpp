#include <string>

#include <esp_log.h>

#include "InterfaceButton.h"

#include "GUI.h"

static InterfaceButton btn{GPIO_NUM_0};

struct Logger {
  static constexpr char LOG_TAG[] = "Btn event";

  Logger(const std::string &msg) : msg(msg) {}

  void operator()(InterfaceButton::eventID id, gpio_num_t pin) {
    ESP_LOGW(LOG_TAG, "%s", msg.c_str());
  }

private:
  const std::string msg;
};

void GUI::init() {
  btn.begin();
  btn.onPush(Logger("Pushed"))
      .onRelease(Logger("Released"))
      .onClick(Logger("Clicked"))
      .onLongPush(Logger("Long pushed"));
}
