#ifndef _NETWORK_H_
#define _NETWORK_H_

#include <memory>

namespace nw {

struct IAdapter;

std::shared_ptr<IAdapter> getSoftAP();
std::shared_ptr<IAdapter> getSTA();

} // namespace nw

#endif /* _NETWORK_H_ */
