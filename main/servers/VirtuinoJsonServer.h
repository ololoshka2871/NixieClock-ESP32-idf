#ifndef _HTTP_SERVER_H_
#define _HTTP_SERVER_H_

#include <cstdint>

struct VirtuinoJsonServer {
  static void start(uint16_t port = 8000);
  static void stop();
};

#endif /* _HTTP_SERVER_H_ */
