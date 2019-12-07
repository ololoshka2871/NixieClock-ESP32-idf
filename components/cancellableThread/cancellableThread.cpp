#include <atomic>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "cancellableThread.h"

using namespace support;

struct cancellableThread::context {
  context(const std::string &Name, uint16_t StackDepth, UBaseType_t Priority)
      : handle{nullptr}, state{READY}, Name(Name), StackDepth(StackDepth),
        Priority(Priority) {}

  TaskHandle_t handle;
  std::atomic<thread_state> state;

  std::string Name;
  uint16_t StackDepth;
  UBaseType_t Priority;

  bool newthread(cancellableThread *parent) {
    return xTaskCreate(cancellableThread::adapter, Name.c_str(), StackDepth,
                       parent, Priority, &handle) == pdPASS;
  }
};

cancellableThread::cancellableThread(const std::string &Name,
                                     uint16_t StackDepth, UBaseType_t Priority)
    : ctx{std::make_unique<context>(Name, StackDepth, Priority)} {
  assert(StackDepth >= configMINIMAL_STACK_SIZE + 1024);
}

cancellableThread::~cancellableThread() { cancel().join(); }

cancellableThread &cancellableThread::cancel() {
  if (ctx->state == RUNNING) {
    ctx->state = CANCELED;
  } else {
    ctx->state = FINISHED;
  }
  return *this;
}

bool cancellableThread::testCancel() const { return ctx->state == CANCELED; }

void cancellableThread::adapter(void *ctx) {
  auto _this = static_cast<cancellableThread *>(ctx);
  _this->Run();
  _this->Cleanup();

  vTaskDelete(nullptr);
}

bool cancellableThread::Start() {
  assert(ctx->state == READY);
  ctx->state = ctx->newthread(this) ? RUNNING : FINISHED;
  return ctx->state == RUNNING;
}

bool cancellableThread::Restart() {
  if (ctx->state == RUNNING) {
    cancel().join();
  }
  return Start();
}

void cancellableThread::Cleanup() {
  ctx->state = FINISHED;
  ctx->handle = nullptr;
}

void cancellableThread::Suspend() { vTaskSuspend(ctx->handle); }

void cancellableThread::Resume() { vTaskResume(ctx->handle); }

void cancellableThread::join() {
  while (ctx->state != FINISHED) {
    taskYIELD();
  }
  ctx->state = READY;
}

bool cancellableThread::isRunning() const { return ctx->state == RUNNING; }

#if (INCLUDE_vTaskDelay == 1)
void cancellableThread::Delay(const TickType_t Delay) { vTaskDelay(Delay); }
#endif
