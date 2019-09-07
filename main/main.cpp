#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "Controller.h"
#include "DataBus.h"
#include "DataPolycy/ic74141ParralelPolcy.h"
#include "Effects/Fade.h"
#include "EncodePolicy/Encoder.h"
#include "SelectorPolicy/Selector.h"

using bus_type = DynamicIndication::DataBus<uint8_t>;

static DynamicIndication::Controller<
    DynamicIndication::ic7414ParralelPolicy<bus_type>,
    DynamicIndication::Selector<bus_type>, DynamicIndication::Encoder,
    DynamicIndication::Effects::Fade>
    di_controller;

extern "C" void app_main(void) { vTaskDelay(1000 / portTICK_PERIOD_MS); }
