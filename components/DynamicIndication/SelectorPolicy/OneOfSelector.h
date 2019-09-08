#ifndef _ONE_OF_SELECTOR_H_
#define _ONE_OF_SELECTOR_H_

#include <memory>

namespace DynamicIndication {
namespace SelectorPolicy {

template <typename T> struct OneOfSelector {
  using bus_type = T;
  using data_type = typename bus_type::data_type;

  OneOfSelector(bus_type &selectorBus) : selectorBus(selectorBus), channel(0) {}

  OneOfSelector(const OneOfSelector &) = delete;

  ~OneOfSelector() { setEnabled(false); }

  static constexpr data_type one() { return data_type(1); }

  uint32_t next_element() {
    channel = (channel + 1) % selectorBus.width();
    if (isEnabled()) {
      selectorBus.setData(one() << channel);
    }
    return channel;
  }

  void setEnabled(bool setEnabled = true) {
    selectorBus.setData(setEnabled ? one() << channel : 0);
  }

  bool isEnabled() const { return !!selectorBus.getData(); }

  uint32_t group_count() const { return selectorBus.width(); }

  bool configured() const { return group_count() > 0; }

  bool isFirstElement() const { return channel == 0; }

private:
  bus_type &selectorBus;
  uint8_t channel;
};

} // namespace SelectorPolicy
} // namespace DynamicIndication

#endif //_ONE_OF_SELECTOR_H_
