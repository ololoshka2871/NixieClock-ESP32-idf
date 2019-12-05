#ifndef _SOFT_AP_H_
#define _SOFT_AP_H_

#include "IAdapter.h"

namespace nw {

struct SoftAP : public IAdapter {
  static constexpr char LOG_TAG[] = "SoftAP";

  ~SoftAP();
  void start() override;
};

} // namespace nw

#endif /* _SOFT_AP_H_ */
