#ifndef _CANCELLABLE_THREAD_H_
#define _CANCELLABLE_THREAD_H_

#include <memory>

#include <freertos/FreeRTOS.h>

namespace support {

struct cancellableThread {
  cancellableThread(const std::string &Name, uint16_t StackDepth,
                    UBaseType_t Priority);
  virtual ~cancellableThread();

  cancellableThread &cancel();
  bool testCancel() const;
  bool Start();
  bool Restart();
  void join();
  bool isRunning() const;

#if (INCLUDE_vTaskSuspend == 1)
  void Suspend();

  void Resume();
#endif

private:
  enum thread_state {
    READY,
    RUNNING,
    CANCELED,
    FINISHED,
  };

  struct context;
  std::unique_ptr<context> ctx;

  static void adapter(void *ctx);

protected:
  virtual void Cleanup();
  virtual void Run() = 0;

#if (INCLUDE_vTaskDelay == 1)
  void Delay(const TickType_t Delay);
#endif
};

} // namespace support

#endif /* _CANCELLABLE_THREAD_H_ */
