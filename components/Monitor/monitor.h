#ifndef MONITOR_H
#define MONITOR_H

#include <cancellableThread.h>

namespace esp {

class Monitor : public support::cancellableThread {
public:
  Monitor();

protected:
  void Run() override;
};

} // namespace esp

#endif // MONITOR_H
