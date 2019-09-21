#ifndef _HTTP_SERVER_H_
#define _HTTP_SERVER_H_

struct HttpServer {
  static void begin();
  static void start();

private:
  static constexpr char GET[] = "GET";

  static void createMdnsRecord();
  static void createSoftAP();
  static void configureRouter();
};

#endif /* _HTTP_SERVER_H_ */
