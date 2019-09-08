#ifndef FADE_H
#define FADE_H

#include <atomic>
#include <vector>

#include "IEffect.h"

namespace DynamicIndication {
namespace Effects {

template <typename T> struct Fade : public IEffect<T> {
  void nextFrame() override { ++frame_counter; }

  void setDestinationData(const std::vector<T> &destination) override {
    animation_src = std::move(animation_dest);
    animation_dest = destination;
    generate_animated_diff();

    frame_counter = 0;
  }

  void setDestinationData(std::vector<T> &&destination) override {
    animation_src = std::move(animation_dest);
    animation_dest = std::move(destination);
    generate_animated_diff();

    frame_counter = 0;
  }

  void initBuffers(size_t fbsize) override {
    animated_diff.resize(fbsize);
    animation_dest.resize(fbsize);
  }

  const std::vector<T> &frame() const override { return animation_dest; }

private:
  std::vector<T> animation_src;
  std::vector<T> animated_diff;
  std::vector<T> animation_dest;

  std::atomic<size_t> frame_counter;

  void generate_animated_diff() {
    auto animation_dest_size = animation_dest.size();
    auto animation_src_size = animation_src.size();

    for (uint32_t i = 0; i < animation_dest_size; ++i) {
      auto src_char = i < animation_src_size ? animation_src.at(i) : T();
      auto dest_char = animation_dest.at(i);
      animated_diff[i] = (src_char != dest_char) ? T() : src_char;
    }
  }
};

} // namespace Effects

} // namespace DynamicIndication

#endif // FADE_H
