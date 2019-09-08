#include <driver/gpio.h>

#include "Nixie.h"
#include "RTC.h"

static RTCManager rtc_ctrl;

extern "C" void app_main(void) {
  gpio_install_isr_service(0); // interrupt for all gpio events

  Nixie::configure();

  rtc_ctrl.loadTime()
      .setCallback(std::bind(Nixie::setValue, std::placeholders::_1))
      .begin();
}
