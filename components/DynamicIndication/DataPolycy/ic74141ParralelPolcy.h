#ifndef _ID1_PARRALEL_POLICY_H_
#define _ID1_PARRALEL_POLICY_H_

#include <memory>
#include <vector>

#include "esp_log.h"

#include "../typed"

namespace DynamicIndication {
namespace DataPolicy {

template <typename T> struct ic7414ParralelPolicy {
  using bus_type = T;
  using data_type = typename bus_type::data_type;
  static constexpr char *TAG = "ic7414ParralelPolicy";

  static constexpr uint32_t bits_pre_channel = 4;

  using framebuffer_element_type = typename typed<bits_pre_channel>::type;

  ic7414ParralelPolicy(bus_type &databus, const uint32_t group_count = 1)
      : databus{databus}, channels_pre_data_bus{databus.width() /
                                                bits_pre_channel},
        group_count{group_count} {}

  ic7414ParralelPolicy(const ic7414ParralelPolicy &) = delete;

  void setData(const std::vector<data_type> &dataSrc, uint8_t group) {
    uint32_t bus_val = 0;
    for (uint ch = 0; ch < channels_pre_data_bus; ++ch) {
      if (dataSrc.size() <= group + ch * group_count) {
        ESP_LOGW(TAG,
                 "dataSrc.size()[%d] <= group[%d] + "
                 "ch[%d] * group_count[%d]",
                 dataSrc.size(), group, ch, group_count);
        return;
      }
      auto d = dataSrc.at(group + ch * group_count);
      bus_val |= d << (bits_pre_channel * ch);
    }
    databus.setData(bus_val);
  }

  size_t framebufferSize() const { return channels_pre_data_bus * group_count; }

private:
  bus_type &databus;
  const uint32_t channels_pre_data_bus;
  const uint32_t group_count;
};

} // namespace DataPolicy
} // namespace DynamicIndication

#endif /* _ID1_PARRALEL_POLICY_H_ */
