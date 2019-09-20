#include <cstring>
#include <thread>

#include <esp_event.h>
#include <esp_log.h>
#include <esp_wifi.h>
#include <freertos/event_groups.h>
#include <mdns.h>
#include <nvs_flash.h>

#include "esp_eth.h"

#include <server/jsl-http.h>

#include "HttpServer.h"

#define AP_STARTED BIT(0)
#define CONNECTED_BITS (AP_STARTED)

static const char LOG_TAG[] = "Http server";
static const char SSID[] = "NIXIE-ESP32";
static const char PASS[] = "iddqdidkfa";

static tcpip_adapter_ip_info_t info{};
static EventGroupHandle_t s_connect_event_group;

/**********************************************************************************************************************/

static void test_target(const jsl_http_common::req_t &_req,
                        jsl_http_common::res_t &_res) {
  ESP_LOGI(LOG_TAG, "test_target called.");
  std::ostringstream &out = _res;

  std::string fname = "/static";
  if (_req.path().size() > 1) {
    fname += "/res";
  }
  fname += "/" + _req.args().at("file");

  ESP_LOGI(LOG_TAG, "Opening file : %s", fname.c_str());

  out << jsl_http_common::dump_path("Path", _req.path());
  out << jsl_http_common::dump_pmap("Args", _req.args());
  out << jsl_http_common::dump_pmap("Query", _req.query());
  _res.write_file("text/plain");
  return;
}

/**********************************************************************************************************************/

static void on_wifi_ap_started(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data) {
  ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
  xEventGroupSetBits(s_connect_event_group, AP_STARTED);
}

static void initialise_mdns(void) {
  mdns_init();
  mdns_hostname_set("Nixie Clock esp32");
  mdns_instance_name_set("mdns0");

  mdns_txt_item_t serviceTxtData[] = {{"board", "esp32"}, {"path", "/"}};

  ESP_ERROR_CHECK(
      mdns_service_add("ESP32-WebServer", "_http", "_tcp", 80, serviceTxtData,
                       sizeof(serviceTxtData) / sizeof(serviceTxtData[0])));
}

static void start() {

  ESP_ERROR_CHECK(tcpip_adapter_dhcps_stop(TCPIP_ADAPTER_IF_AP));

  IP4_ADDR(&info.ip, 192, 168, 35, 1);
  IP4_ADDR(&info.gw, 192, 168, 35, 1);
  IP4_ADDR(&info.netmask, 255, 255, 255, 0);
  ESP_ERROR_CHECK(tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_AP, &info));

  wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));
  ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));

  ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_AP_START,
                                             &on_wifi_ap_started, nullptr));

  wifi_config_t ap_config = {
      {
          {},
          {},
          std::strlen(SSID),
          0,
          WIFI_AUTH_WPA_WPA2_PSK,
          0,
          1,
      },
  };
  std::strncpy((char *)ap_config.ap.ssid, SSID, 32);
  std::strncpy((char *)ap_config.ap.password, PASS, 64);
  if (strlen(PASS) == 0) {
    ap_config.ap.authmode = WIFI_AUTH_OPEN;
  }

  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap_config));
  ESP_ERROR_CHECK(esp_wifi_start());

  ESP_ERROR_CHECK(tcpip_adapter_dhcps_start(TCPIP_ADAPTER_IF_AP));
}

static esp_err_t create_ap(void) {
  if (s_connect_event_group != NULL) {
    return ESP_ERR_INVALID_STATE;
  }
  s_connect_event_group = xEventGroupCreate();
  start();
  xEventGroupWaitBits(s_connect_event_group, CONNECTED_BITS, true, true,
                      portMAX_DELAY);
  ESP_LOGI(LOG_TAG, "AP created");
  ESP_LOGI(LOG_TAG, "IPv4 address: " IPSTR, IP2STR(&info.ip));

  return ESP_OK;
}

void HttpServerConfigure() {
  static const char GET[] = "GET";

  jsl_http::addRoute(GET, "/{file}", test_target);

  ESP_ERROR_CHECK(nvs_flash_init());
  tcpip_adapter_init();
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  initialise_mdns();

  ESP_ERROR_CHECK(create_ap());

  jsl_http::start(s_connect_event_group);
}

void HttpServerStart() {
  std::thread httpserver{&jsl_http::run, nullptr};
  httpserver.detach();
}
