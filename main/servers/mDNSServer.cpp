
#include <mdns.h>

#include "mDNSServer.h"

static constexpr char mDNSServerName[] = "mdns0";
static constexpr char mDNSHostName[] = "nixie-esp32";

mDNSServer *mDNSServer::inst;

mDNSServer &mDNSServer::instance() {
  if (!inst) {
    inst = new mDNSServer;
  }
  return *inst;
}

mDNSServer &
mDNSServer::addService(const char *service_name, const char *service,
                       const char *proto, uint16_t port,
                       const std::vector<mdns_txt_item_t> &txtData) {
  ESP_ERROR_CHECK(mdns_service_add(
      service_name, service, proto, port,
      const_cast<mdns_txt_item_t *>(txtData.data()), txtData.size()));
  return *this;
}

mDNSServer::mDNSServer() {
  ESP_ERROR_CHECK(mdns_init());
  ESP_ERROR_CHECK(mdns_hostname_set(mDNSHostName));
  ESP_ERROR_CHECK(mdns_instance_name_set(mDNSServerName));
}
