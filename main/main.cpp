#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "Controller.h"
#include "DataPolycy/ic74141ParralelPolcy.h"
#include "Effects/Fade.h"
#include "EncodePolicy/ic74141Encoder.h"
#include "Output/ParalelOutputBus.h"
#include "SelectorPolicy/OneOfSelector.h"

#define ENABLE_PINS 1

using encoder_type = DynamicIndication::EncodePolicy::ic74141Encoder<uint8_t>;
using effector_type =
    DynamicIndication::Effects::Fade<typename encoder_type::Output_t>;

static DynamicIndication::Output::ParalelOutputBus<8> DataBus {
#if ENABLE_PINS
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
#if ENABLE_PINS
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

static effector_type fade_effector;

static DynamicIndication::Controller di_controller(datapolicy, selector,
                                                   encoder, &fade_effector);

extern "C" void app_main(void) {
  di_controller.setUpdateInterval(5000).setText("000000").setEnabled();

  vTaskDelay(1000 / portTICK_PERIOD_MS);
}
