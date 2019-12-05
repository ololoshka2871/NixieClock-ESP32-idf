#include <algorithm>
#include <chrono>
#include <thread>
#include <vector>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "monitor.h"

#define LOG(fmt, ...) printf(fmt "\n", ##__VA_ARGS__)

using namespace esp;

static constexpr char _task_state_to_char(const eTaskState state) {
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

void Monitor::Run() {
  using namespace std::chrono_literals;

  std::vector<TaskStatus_t> taskdata;
  while (!testCancel()) {
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

    LOG("FreeRTOS threadinfo:");

    // Avoid divide by zero errors.
    if (ulTotalRunTime > 0) {
      std::for_each(
          taskdata.cbegin(), taskdata.cend(), [ulTotalRunTime](auto &taskinfo) {
            if (taskinfo.pcTaskName) {
              auto ulStatsAsPercentage =
                  taskinfo.ulRunTimeCounter / ulTotalRunTime;

              LOG("%20s: %c, P:%u, Sf:%6u, %2u%% (%u t)", taskinfo.pcTaskName,
                  _task_state_to_char(taskinfo.eCurrentState),
                  taskinfo.uxCurrentPriority, taskinfo.usStackHighWaterMark,
                  ulStatsAsPercentage, taskinfo.ulRunTimeCounter);
            }
          });
    }

    LOG("");
    LOG("Current Heap Free Size: %u", xPortGetFreeHeapSize());
    LOG("Minimal Heap Free Size: %u", xPortGetMinimumEverFreeHeapSize());
    LOG("");

    std::this_thread::sleep_for(1s);
  }
}

Monitor::Monitor()
    : support::cancellableThread{"Monitor", configMINIMAL_STACK_SIZE + 1100,
                                 tskIDLE_PRIORITY + 1} {}
