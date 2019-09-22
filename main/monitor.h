#ifndef MONITOR_H
#define MONITOR_H

struct Monitor {
  Monitor() = delete;

  static void start(int period_s = 1);

private:
  static void Run(void *_);

  static constexpr char _task_state_to_char(const eTaskState state);
};

#endif // MONITOR_H
