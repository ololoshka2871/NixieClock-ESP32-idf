#ifndef _NET_ADAPTER_H_
#define _NET_ADAPTER_H_

#include <string>

#include <freertos/event_groups.h>

#include <esp_event.h>

struct NetAdapter {
  static constexpr char LOG_TAG[] = "NetAdapter";

  static NetAdapter &instance();

  esp_err_t createSoftap(const std::string &SSID = "esp32",
                         const std::string &PASS = std::string(),
                         bool isHudden = false);
  esp_err_t tryConnect(const std::string &SSID = "esp32",
                       const std::string &PASS = std::string());

private:
  NetAdapter();

  static NetAdapter *inst;

  EventGroupHandle_t event_group;

  tcpip_adapter_ip_info_t ApIPInfo;
  ip4_addr_t sta_ip;

  bool ap_started;
  bool connected;

  void initStorage();
  void initTcpip();
  void initEventLoop();

  void enableSoftAp();
  void setApIP();

  static void on_wifi_ap_started(void *arg, esp_event_base_t event_base,
                                 int32_t event_id, void *event_data);
  static void on_wifi_got_ip(void *arg, esp_event_base_t event_base,
                             int32_t event_id, void *event_data);

  void resetWiFi();
  void enableSTA();

  esp_err_t disconnect();
};

#endif /* _NET_ADAPTER_H_ */
