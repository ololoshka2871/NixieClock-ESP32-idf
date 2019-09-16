#include "Controller.h"
#include "DataPolycy/ic74141ParralelPolcy.h"
#include "Effects/Fade.h"
#include "EncodePolicy/ic74141Encoder.h"
#include "Output/ParalelOutputBus.h"
#include "SelectorPolicy/OneOfSelector.h"

#include "Nixie.h"

#define FORCE_ENABLE_PINS 0

static constexpr uint32_t MICROSECONDS_IN_SECOND = 1000 * 1000;
static constexpr uint32_t FPS = 400;
static constexpr uint32_t ANIMATION_FRAME_COUNT = FPS / 2;

using encoder_type = DynamicIndication::EncodePolicy::ic74141Encoder<uint8_t>;
using effector_type =
    DynamicIndication::Effects::Fade<typename encoder_type::Output_t>;

static DynamicIndication::Output::ParalelOutputBus<8> DataBus {
#if !defined(TEST_MODE) || FORCE_ENABLE_PINS
  GPIO_NUM_27,     // D1A
      GPIO_NUM_26, // D2A
      GPIO_NUM_25, // D3A
      GPIO_NUM_33, // D4A
      GPIO_NUM_32, // D1B
      GPIO_NUM_5,  // D2B
      GPIO_NUM_18, // D3B
      GPIO_NUM_19, // D4B
#else
  GPIO_NUM_NC, GPIO_NUM_NC, GPIO_NUM_NC, GPIO_NUM_NC, GPIO_NUM_NC, GPIO_NUM_NC,
      GPIO_NUM_NC, GPIO_NUM_NC,
#endif
};

static DynamicIndication::Output::ParalelOutputBus<3> SelectorBus {
#if !defined(TEST_MODE) || FORCE_ENABLE_PINS
  GPIO_NUM_13,     // TG0
      GPIO_NUM_2,  // TG1
      GPIO_NUM_14, // TG2
#else
  GPIO_NUM_NC, GPIO_NUM_NC, GPIO_NUM_NC,
#endif
};

static DynamicIndication::DataPolicy::ic7414ParralelPolicy<decltype(DataBus)>
    datapolicy{DataBus, decltype(SelectorBus)::size};
static DynamicIndication::SelectorPolicy::OneOfSelector<decltype(SelectorBus)>
    selector{SelectorBus};
static encoder_type encoder{
    // 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | off > На лампе
    // 3 | 2 | 4 | 1 | 8 | 13| 5 | 12| 9 | 0 | 10  > На входе
    10,
    {
        {'0', 3},
        {'1', 2},
        {'2', 4},
        {'3', 1},
        {'4', 8},
        {'5', 13},
        {'6', 5},
        {'7', 12},
        {'8', 9},
        {'9', 0},
    }};

static effector_type fade_effector(encoder.getunknownCharValue());

DynamicIndication::Controller di_controller(datapolicy, selector, encoder,
                                            &fade_effector);

void Nixie::configure() {
  fade_effector.setPulseWeigth(1.0f).SetAnimationDuration(
      ANIMATION_FRAME_COUNT);
  di_controller
      .setUpdateInterval(MICROSECONDS_IN_SECOND / FPS / SelectorBus.width() - 1)
      .setText("      ")
      .setEnabled();
  di_controller.getEffector();
}

void Nixie::setValue(const std::string &v) { di_controller.setText(v); }
