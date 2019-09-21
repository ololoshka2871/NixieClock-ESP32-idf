#ifndef _M_DNS_SERVER_H_
#define _M_DNS_SERVER_H_

#include <vector>

#include <mdns.h>

/// Это по факту служба Zeroconf/Bonjur
/// На ведре использовать приложуху навроде de.wellenvogel.bonjourbrowser
/// Адрес будет nixie-esp32.local
struct mDNSServer {
  static mDNSServer &instance();

  mDNSServer &addService(const char *service_name, const char *service,
                         const char *proto, uint16_t port,
                         const std::vector<mdns_txt_item_t> &txtData);

private:
  mDNSServer();

  static mDNSServer *inst;
};

#endif /* _M_DNS_SERVER_H_ */
