#ifndef _VSENS_CHANNEL_H_
#define _VSENS_CHANNEL_H_

#include <driver/adc.h>

struct VSensChanel {
  constexpr VSensChanel(adc1_channel_t AdcChannel, float R1, float R2)
      : AdcChannel(AdcChannel), R1(R1), R2(R2) {}

  float raw2voltage(float raw_value) const {
    return raw_value / R2 * (R1 + R2);
  }

  const adc1_channel_t AdcChannel;
  const float R1;
  const float R2;
};

#endif /* _VSENS_CHANNEL_H_ */
