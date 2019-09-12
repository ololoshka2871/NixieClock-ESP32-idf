#ifndef IEFFECT_H
#define IEFFECT_H

#include <cstdint>
#include <functional>
#include <vector>

namespace DynamicIndication {
namespace Effects {

template <typename T> struct IEffect {
  virtual ~IEffect() = default;

  virtual void SetAnimationDuration(size_t FullAnimationFrameCount) = 0;

  virtual void nextFrame() = 0;

  virtual void setDestinationData(const std::vector<T> &destination) = 0;

  virtual void setDestinationData(std::vector<T> &&destination) = 0;

  virtual void initBuffers(size_t fbsize) = 0;

  virtual void
  frameOp(const std::function<void(const std::vector<T> &)> &op) const = 0;
};

} // namespace Effects

} // namespace DynamicIndication

#endif // IEFFECT_H
