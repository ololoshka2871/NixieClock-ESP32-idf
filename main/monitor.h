#ifndef MONITOR_H
#define MONITOR_H

#include <thread>

namespace app {
class Monitor : public std::thread {
public:
  Monitor();

private:
  void Run();
};
} // namespace app

#endif // MONITOR_H
