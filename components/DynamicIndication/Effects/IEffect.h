#ifndef IEFFECT_H
#define IEFFECT_H

#include <cstdint>
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

  virtual const std::vector<T> &frame() const = 0;
};

} // namespace Effects

} // namespace DynamicIndication

#endif // IEFFECT_H
