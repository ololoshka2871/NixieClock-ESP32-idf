#ifndef _CRYTICAL_SECTION_H_
#define _CRYTICAL_SECTION_H_

#include <freertos/FreeRTOS.h>

namespace support {

struct CryticalSection {
  CryticalSection() : state(portENTER_CRITICAL_NESTED()) {}
  ~CryticalSection() { portEXIT_CRITICAL_NESTED(state); }

private:
  int state;
};

} // namespace support

#endif /* _CRYTICAL_SECTION_H_ */
