#ifndef _NIXIE_H_
#define _NIXIE_H_

#include <string>

namespace Nixie {
void configure();
void setValue(const std::string &v);
}; // namespace Nixie

#endif /*_NIXIE_H_*/
