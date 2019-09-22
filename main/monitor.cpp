#include <algorithm>
#include <vector>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <esp_log.h>

#include "monitor.h"

#define MONITOR_PRINTF(...) ESP_LOGW(LOG_TAG, __VA_ARGS__)

static constexpr char LOG_TAG[] = "Thread monitor";

void Monitor::start(int period_s) {
  xTaskCreate(&Monitor::Run, "monitor", 2048,
              reinterpret_cast<void *>(period_s), 10, nullptr);
}

void Monitor::Run(void *period_i) {
  const TickType_t period =
      pdMS_TO_TICKS(reinterpret_cast<int>(period_i) * 1000);

  std::vector<TaskStatus_t> taskdata;

  while (true) {
    taskdata.resize(uxTaskGetNumberOfTasks());
    uint32_t ulTotalRunTime;

    uxTaskGetSystemState(taskdata.data(), taskdata.size(), &ulTotalRunTime);

    // For percentage calculations.
    ulTotalRunTime /= 100UL;

    // sort
    std::sort(taskdata.begin(), taskdata.end(),
              [](const TaskStatus_t &a, const TaskStatus_t &b) {
                return a.xTaskNumber < b.xTaskNumber;
              });

    MONITOR_PRINTF("FreeRTOS threadinfo:");
    // Avoid divide by zero errors.
    if (ulTotalRunTime > 0) {
      std::for_each(
          taskdata.cbegin(), taskdata.cend(), [ulTotalRunTime](auto &taskinfo) {
            auto ulStatsAsPercentage =
                taskinfo.ulRunTimeCounter / ulTotalRunTime;
            MONITOR_PRINTF(
                "%20s: %c, P:%u, Sf:%6u, %u%% (%u t)", taskinfo.pcTaskName,
                _task_state_to_char(taskinfo.eCurrentState),
                taskinfo.uxCurrentPriority, taskinfo.usStackHighWaterMark,
                ulStatsAsPercentage, taskinfo.ulRunTimeCounter);
          });
    }

    MONITOR_PRINTF("");
    MONITOR_PRINTF("Current Heap Free Size: %u", xPortGetFreeHeapSize());
    MONITOR_PRINTF("Minimal Heap Free Size: %u",
                   xPortGetMinimumEverFreeHeapSize());
    MONITOR_PRINTF("");

    vTaskDelay(period);
  }
}

constexpr char Monitor::_task_state_to_char(const eTaskState state) {
  switch (state) {
  case eRunning:
    return '*';
  case eReady:
    return 'R';
  case eBlocked:
    return 'B';
  case eSuspended:
    return 'S';
  case eDeleted:
    return 'D';
  default:
    return 0x00;
  }
}
