#ifndef _VSENSORS_H_
#define _VSENSORS_H_

#include <vector>

#include "VSensChanel.h"

struct VSensors {
  VSensors(std::initializer_list<VSensChanel> channels_def);

  float getChannelVoltage(size_t channel);

private:
  std::vector<VSensChanel> channels;
};

#endif /* _VSENSORS_H_ */
