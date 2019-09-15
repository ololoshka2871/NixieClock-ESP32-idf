#ifndef _FAKE_ARDUINO_H_
#define _FAKE_ARDUINO_H_

#include "soc/gpio_reg.h"
#include "esp32-hal.h"

#define digitalPinToPort(pin)       (((pin)>31)?1:0)
#define digitalPinToBitMask(pin)    (1UL << (((pin)>31)?((pin)-32):(pin)))

#define portOutputRegister(port)    ((volatile uint32_t*)((port)?GPIO_OUT1_REG:GPIO_OUT_REG))
#define portInputRegister(port)     ((volatile uint32_t*)((port)?GPIO_IN1_REG:GPIO_IN_REG))

#endif /* _FAKE_ARDUINO_H_ */
