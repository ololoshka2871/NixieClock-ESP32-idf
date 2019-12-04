#ifndef _HTTP_SERVER_H_
#define _HTTP_SERVER_H_

#include <cstdint>

#include <lwip/err.h>

struct netconn;

struct VirtuinoJsonServer {
  static void begin();
  static void start(uint16_t port = 8000);

private:
  static void createMdnsRecord();
  static void createSoftAP();
  static void configureRouter();

  static void Run(void *ctx);

  static void process_clientConnection(netconn &clientConnection);
};

#endif /* _HTTP_SERVER_H_ */
