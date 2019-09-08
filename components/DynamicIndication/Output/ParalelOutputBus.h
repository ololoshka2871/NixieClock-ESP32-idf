#ifndef _PARALEL_OUTPUT_BUS_H_
#define _PARALEL_OUTPUT_BUS_H_

#include <algorithm>
#include <cassert>
#include <type_traits>
#include <vector>

#include <driver/gpio.h>

#include "../typed"

namespace DynamicIndication {
namespace Output {

template <unsigned int _size> struct ParalelOutputBus {
  static constexpr size_t size = _size;

  using data_type = typename typed<_size>::type;

  static_assert(!std::numeric_limits<data_type>::is_signed,
                "Tv mast be unsigned");
  static_assert(std::numeric_limits<data_type>::is_integer,
                "Tv mast be integer");

  ParalelOutputBus(std::initializer_list<gpio_num_t> pins, bool invert = false)
      : pinsNumbers{pins}, invert{invert} {

    assert(pins.size() <= sizeof(data_type) * 8);

    for (auto it = pinsNumbers.cbegin(); it != pinsNumbers.cend(); ++it) {
      auto pn = *it;
      if (pn != GPIO_NUM_NC) {
        gpio_reset_pin(pn);
        gpio_set_direction(pn, GPIO_MODE_OUTPUT);
        gpio_set_pull_mode(pn, GPIO_FLOATING);
      }
    }
    setData(invert ? std::numeric_limits<data_type>::max() : 0);
  }

  ParalelOutputBus(const ParalelOutputBus &) = delete;

  ~ParalelOutputBus() {
    std::for_each(pinsNumbers.cbegin(), pinsNumbers.cend(), [](gpio_num_t pn) {
      if (pn != GPIO_NUM_NC)
        gpio_reset_pin(pn);
    });
  }

  void setData(data_type data) {
    value = data;
    for (std::size_t i = 0; i < pinsNumbers.size(); ++i) {
      auto pn = pinsNumbers[i];
      if (pn != GPIO_NUM_NC) {
        gpio_set_level(pn, (((data & 1u << i) != 0) ^ invert));
      }
    }
  }

  data_type getData() const { return value; }

  int width() const { return pinsNumbers.size(); }

  void setInverted(bool inverted) {
    if (inverted != invert) {
      auto oldata = getData();
      invert = inverted;
      setData(oldata);
    }
  }

  void setInvert(bool invert = true) { this->invert = invert; }
  bool isInverted() const { return invert; }

private:
  std::vector<gpio_num_t> pinsNumbers;
  data_type value;
  bool invert;
};

} // namespace Output
} // namespace DynamicIndication

#endif //_PARALEL_OUTPUT_BUS_H_
