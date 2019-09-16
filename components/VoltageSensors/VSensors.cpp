#include <algorithm>
#include <cmath>

#include <driver/adc.h>

#include "VSensors.h"

static constexpr float ADC_REFERENCE = 1.1f; // TODO
static constexpr float ADC_ONE_Voltage = ADC_REFERENCE / 1024.0f;

VSensors::VSensors(std::initializer_list<VSensChanel> channels_def)
    : channels(channels_def) {
  adc1_config_width(ADC_WIDTH_BIT_10);
  std::for_each(channels.begin(), channels.end(), [](VSensChanel &vs) {
    ESP_ERROR_CHECK(adc1_config_channel_atten(vs.AdcChannel, ADC_ATTEN_DB_0));
  });
}

float VSensors::getChannelVoltage(size_t channel) {
  if (channel >= channels.size()) {
    return 0.0f;
  }

  auto &ch = channels.at(channel);
  auto val = adc1_get_raw(ch.AdcChannel);
  return ch.raw2voltage(val * ADC_ONE_Voltage);
}
