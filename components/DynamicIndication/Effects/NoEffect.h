#ifndef NO_EFFECT_H
#define NO_EFFECT_H

#include <algorithm>
#include <vector>

#include "IEffect.h"

namespace DynamicIndication {
namespace Effects {

template <typename T> struct NoEffect : public IEffect<T> {
  void SetAnimationDuration(size_t FullAnimationFrameCount) override {}
  void nextFrame() override {}

  void setDestinationData(const std::vector<T> &destination) override {
    std::copy(destination.cbegin(), destination.cend(), data.begin());
    if (destination.size() < data.size()) {
      auto it = data.end();
      std::advance(it, -(data.size() - destination.size() + 1));
      std::for_each(it, data.end(), [](T &element) { element = T(); });
    }
  }

  void setDestinationData(std::vector<T> &&destination) override {
    auto src_size = destination.size();
    data = std::move(destination);
    if (src_size < data.size()) {
      auto it = data.end();
      std::advance(it, -(data.size() - src_size + 1));
      std::for_each(it, data.end(), [](T &element) { element = T(); });
    }
  }

  void initBuffers(size_t fbsize) override { data.resize(fbsize); }

  std::vector<T> &frame() const override { return data; }

private:
  std::vector<T> data;
};

} // namespace Effects

} // namespace DynamicIndication

#endif // NO_EFFECT_H
