#include <cstdint>
#include <cstring>

#include <esp_eth.h>
#include <esp_log.h>
#include <esp_wifi.h>
#include <nvs_flash.h>

#include "NetAdapter.h"

#define sizeof_member(s, member) (sizeof(((s *)0)->member))

static constexpr EventBits_t AP_STARTED = BIT0;
static constexpr EventBits_t STA_CONNECTED = BIT1;
static constexpr EventBits_t STA_DISCONNECTED = BIT2;

NetAdapter *NetAdapter::inst;

NetAdapter &NetAdapter::instance() {
  if (!inst) {
    inst = new NetAdapter();
  }
  return *inst;
}

void NetAdapter::on_wifi_ap_started(void *arg, esp_event_base_t event_base,
                                    int32_t event_id, void *event_data) {
  NetAdapter *_this = static_cast<NetAdapter *>(arg);
  // ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
  xEventGroupSetBits(_this->event_group, AP_STARTED);
}

void NetAdapter::on_wifi_got_ip(void *arg, esp_event_base_t event_base,
                                int32_t event_id, void *event_data) {
  NetAdapter *_this = static_cast<NetAdapter *>(arg);
  ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
  memcpy(&_this->sta_ip, &event->ip_info.ip, sizeof(ip4_addr_t));
  xEventGroupSetBits(_this->event_group, STA_CONNECTED);
}

void NetAdapter::resetWiFi() {
  wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));
  ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
}

void NetAdapter::enableSoftAp() {
  wifi_mode_t current_mode;
  ESP_ERROR_CHECK(esp_wifi_get_mode(&current_mode));
  ESP_ERROR_CHECK(
      esp_wifi_set_mode(static_cast<wifi_mode_t>(current_mode | WIFI_MODE_AP)));
}

void NetAdapter::setApIP() {
  ESP_ERROR_CHECK(tcpip_adapter_dhcps_stop(TCPIP_ADAPTER_IF_AP));
  ESP_ERROR_CHECK(tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_AP, &ApIPInfo));
  ESP_ERROR_CHECK(tcpip_adapter_dhcps_start(TCPIP_ADAPTER_IF_AP));
}

esp_err_t NetAdapter::createSoftap(const std::string &SSID,
                                   const std::string &PASS, bool isHudden) {
  if (ap_started) {
    return ESP_ERR_INVALID_STATE;
  }

  enableSoftAp();

  ESP_ERROR_CHECK(esp_event_handler_register(
      WIFI_EVENT, WIFI_EVENT_AP_START, &NetAdapter::on_wifi_ap_started, this));

  wifi_config_t ap_config = {
      {{},
       {},
       static_cast<uint8_t>(SSID.size()),
       0,
       (PASS.size() == 0) ? WIFI_AUTH_OPEN : WIFI_AUTH_WPA_WPA2_PSK,
       static_cast<u_int8_t>(isHudden),
       1}};

  std::strncpy((char *)ap_config.ap.ssid, SSID.c_str(),
               sizeof_member(wifi_ap_config_t, ssid));
  std::strncpy((char *)ap_config.ap.password, PASS.c_str(),
               sizeof_member(wifi_ap_config_t, password));

  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap_config));
  ESP_ERROR_CHECK(esp_wifi_start());

  xEventGroupWaitBits(event_group, AP_STARTED, true, true, portMAX_DELAY);

  setApIP();
  ap_started = true;

  ESP_LOGI(LOG_TAG, "siftAP created, IPv4 address: " IPSTR,
           IP2STR(&ApIPInfo.ip));

  return ESP_OK;
}

void NetAdapter::enableSTA() {
  wifi_mode_t current_mode;
  ESP_ERROR_CHECK(esp_wifi_get_mode(&current_mode));
  ESP_ERROR_CHECK(esp_wifi_set_mode(
      static_cast<wifi_mode_t>(current_mode | WIFI_MODE_STA)));
}

esp_err_t NetAdapter::disconnect() {
  auto disconnect_handler = [](void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data) {
    NetAdapter *_this = static_cast<NetAdapter *>(arg);
    xEventGroupSetBits(_this->event_group, STA_DISCONNECTED);
  };

  ESP_ERROR_CHECK(esp_event_handler_register(
      WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, disconnect_handler, this));
  auto res =
      xEventGroupWaitBits(event_group, STA_DISCONNECTED, true, true, 1500);
  if (res & STA_DISCONNECTED) {
    ESP_ERROR_CHECK(esp_event_handler_unregister(
        WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, disconnect_handler));
    return ESP_OK;
  } else {
    return ESP_ERR_TIMEOUT;
  }
}

esp_err_t NetAdapter::tryConnect(const std::string &SSID,
                                 const std::string &PASS) {
  if (connected) {
    ESP_LOGW(LOG_TAG, "Reconnecting wifi to %s", SSID.c_str());
    auto res = disconnect();
    if (res != ESP_OK) {
      return res;
    }
  }

  enableSTA();

  ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                             &on_wifi_got_ip, this));
  wifi_config_t wifi_config{};
  std::strncpy((char *)wifi_config.sta.ssid, SSID.c_str(),
               sizeof_member(wifi_sta_config_t, ssid));
  std::strncpy((char *)wifi_config.sta.password, PASS.c_str(),
               sizeof_member(wifi_sta_config_t, password));

  ESP_LOGI(LOG_TAG, "Connecting to %s...", wifi_config.sta.ssid);
  ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());
  ESP_ERROR_CHECK(esp_wifi_connect());

  auto wr = xEventGroupWaitBits(event_group, STA_CONNECTED, true, true, 1500);
  if (wr & STA_CONNECTED) {
    ESP_LOGI(LOG_TAG, "Connected to %s", SSID.c_str());
    ESP_LOGI(LOG_TAG, "IPv4 address: " IPSTR, IP2STR(&sta_ip));
    connected = true;
    return ESP_OK;
  }
  ESP_LOGE(LOG_TAG, "Failed to connect to %s, timeout", SSID.c_str());
  esp_wifi_disconnect();
  return ESP_ERR_TIMEOUT;
}

NetAdapter::NetAdapter() : ap_started(false), connected(false) {
  initStorage();
  initTcpip();
  initEventLoop();
  resetWiFi();
}

void NetAdapter::initStorage() { ESP_ERROR_CHECK(nvs_flash_init()); }

void NetAdapter::initTcpip() {
  tcpip_adapter_init();

  IP4_ADDR(&ApIPInfo.ip, 192, 168, 35, 1);
  IP4_ADDR(&ApIPInfo.gw, 192, 168, 35, 1);
  IP4_ADDR(&ApIPInfo.netmask, 255, 255, 255, 0);
}

void NetAdapter::initEventLoop() {
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  event_group = xEventGroupCreate();
}
