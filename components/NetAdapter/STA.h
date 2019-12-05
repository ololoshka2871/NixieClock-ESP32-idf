#ifndef _STA_H_
#define _STA_H_

#include "IAdapter.h"

namespace nw {

struct STA : public IAdapter {
  void start() override;
};

} // namespace nw

#endif /* _STA_H_ */
