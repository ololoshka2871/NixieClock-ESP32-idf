#ifndef _WIFI_ENABLER_H_
#define _WIFI_ENABLER_H_

#include "AbstractGUIState.h"

struct WifiEnabler : public AbstractGUIState {
  uint64_t idleTimeout() const override { return 10_s; }
  const char *getLOG_TAG() const override { return LOG_TAG; }

private:
  static constexpr char LOG_TAG[] = "WifiEnabler";
};

#endif /* _WIFI_ENABLER_H_ */
