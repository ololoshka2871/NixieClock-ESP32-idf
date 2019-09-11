#ifndef FADE_H
#define FADE_H

#include <algorithm>
#include <atomic>
#include <cmath>
#include <limits>
#include <vector>

#include "IEffect.h"

#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include "esp_log.h"

#define GRADATIONS_FROM_FRAME_COUNT(fc)                                        \
  (fc) / 24 // +1 dradation pre 24 animation frames

#define MINIMAL_ANIMATION_GRADATIONS_COUNT 3u

namespace DynamicIndication {
namespace Effects {

static constexpr char *TAG = "MyModule";

template <typename T> struct Fade : public IEffect<T> {
  using this_type = Fade<T>;

  /// Используем следующее уравнение для опрределения яркости в точке анимации
  /// @ref halfAnimationDuration = FullAnimationFrameCount / 2
  /// Для убывающей части: F1(frame_counter) = halfAnimationDuration -
  /// frame_counter Для возрастающей части: F2(frame_counter) =
  /// -halfAnimationDuration + frame_counter
  ///
  /// Если на текущем кадре мы зажгли цифруы (@ref animation_src или @ref
  /// animation_dest), то увеличим интергальную переменную @ref ShowIntegrator
  /// на halfAnimationDuration. иначе не увеличиваем.
  ///
  /// Для определения следует ли зажечь цыфру или погасить используем следующий
  /// критерий:
  ///
  /// integrate (F(frame_counter)) dx from 0 to frame_counter <
  /// @ref IntergalhowedLight
  ///
  /// [Интегральная сумма яркости, которая должна быть отображена в идеале,
  /// меньше чем фактически отображенная] То в текущем кадре показать цифру
  ///
  /// Для второй половины просто меняем функцию и пределы интегрирования

  Fade(const T spaceChar) : PulseWeigth(1.0f), spaceChar(spaceChar) {}

  /**
   * @brief Функция яркости символа от номера кадра анимации
   * @param C Длина анимации
   * @param frame_number номер кадра
   * @return яркость в пределах [0..C]
   */
  static constexpr size_t F(size_t C, size_t frame_number) {
    return C - frame_number;
  }

  void SetAnimationDuration(size_t FullAnimationFrameCount) override {
    this->FullAnimationFrameCount = FullAnimationFrameCount;
    halfAnimationDuration = FullAnimationFrameCount / 2;
  }

  void nextFrame() override {
    if (frame_counter < halfAnimationDuration) {
      // убывание яркости
      IntegralDestLight += F(halfAnimationDuration, frame_counter);
      if (IntergalhowedLight < IntegralDestLight) {
        // показать символы
        selected_buf = &animation_src;
        IntergalhowedLight += halfAnimationDuration * PulseWeigth;
      } else {
        // не показывать
        selected_buf = &animated_diff;
      }
      ++frame_counter;
    } else if (frame_counter < FullAnimationFrameCount) {
      IntegralDestLight +=
          F(halfAnimationDuration, FullAnimationFrameCount - frame_counter);
      if (IntergalhowedLight < IntegralDestLight) {
        // показать символы
        selected_buf = &animation_dest;
        IntergalhowedLight += halfAnimationDuration * PulseWeigth;
      } else {
        // не показывать
        selected_buf = &animated_diff;
      }
      ++frame_counter;
    } else {
      selected_buf = &animation_dest;
    }
  }

  void setDestinationData(const std::vector<T> &destination) override {
    animation_src = std::move(animation_dest);
    animation_dest = destination;
    generate_animated_diff();

    reset_animation();
  }

  void setDestinationData(std::vector<T> &&destination) override {
    animation_src = std::move(animation_dest);
    animation_dest = std::move(destination);
    generate_animated_diff();

    reset_animation();
  }

  void initBuffers(size_t fbsize) override {
    animated_diff.resize(fbsize);
    animation_dest.resize(fbsize);
  }

  const std::vector<T> &frame() const override { return *selected_buf; }

  this_type &setPulseWeigth(float weigth) {
    PulseWeigth = weigth;
    return *this;
  }

private:
  std::vector<T> animation_src;
  std::vector<T> animated_diff;
  std::vector<T> animation_dest;

  std::vector<T> *selected_buf;

  size_t FullAnimationFrameCount;
  size_t halfAnimationDuration;

  float PulseWeigth;

  float IntergalhowedLight;
  size_t IntegralDestLight;

  const T spaceChar;

  std::atomic<size_t> frame_counter;

  void generate_animated_diff() {
    auto animation_dest_size = animation_dest.size();
    auto animation_src_size = animation_src.size();

    for (uint32_t i = 0; i < animation_dest_size; ++i) {
      auto src_char = i < animation_src_size ? animation_src.at(i) : T();
      auto dest_char = animation_dest.at(i);
      animated_diff[i] = (src_char != dest_char) ? spaceChar : src_char;
    }
  }

  void reset_animation() {
    selected_buf = &animation_src;
    frame_counter = 0;
    IntergalhowedLight = 0;
    IntegralDestLight = 0;
  }
};

} // namespace Effects
} // namespace DynamicIndication

#endif // FADE_H
