#include <esp_log.h>

#include <server/jsl-http.h>

#include "NetAdapter.h"
#include "mDNSServer.h"

#include "HttpServer.h"

static constexpr char _ssid[] = "DIR-300NRU";
static constexpr char _pass[] = "*Bu@#?Kz,u.{-guG";

static const char LOG_TAG[] = "Http server";
static const char SSID[] = "NIXIE-ESP32";
static const char PASS[] = "iddqdidkfa";

/**********************************************************************************************************************/

static void test_target(const jsl_http_common::req_t &_req,
                        jsl_http_common::res_t &_res) {
  ESP_LOGI(LOG_TAG, "test_target called.");
  std::ostringstream &out = _res;

  out << jsl_http_common::dump_path("Path", _req.path());
  out << jsl_http_common::dump_pmap("Args", _req.args());
  out << jsl_http_common::dump_pmap("Query", _req.query());
  _res.write_file("text/plain");
  return;
}

/**********************************************************************************************************************/

void HttpServer::begin() {
  createSoftAP();
  createMdnsRecord();

  configureRouter();
}

void HttpServer::start() {
  // не через std::thread, чтобы задать размер стека
  xTaskCreate(&jsl_http::run, "HttpServer", 4096, nullptr, 10, nullptr);
}

void HttpServer::createMdnsRecord() {
  mDNSServer::instance().addService("ESP32-WebServer", "_http", "_tcp", 80,
                                    {{"board", "esp32"}, {"path", "/"}});
}

void HttpServer::createSoftAP() {
  if (_ssid) {
    if (NetAdapter::instance().tryConnect(_ssid, _pass) == ESP_OK) {
      return;
    }
  }
  NetAdapter::instance().createSoftap(SSID, PASS);
}

void HttpServer::configureRouter() {
  jsl_http::addRoute(GET, "/", test_target);
  jsl_http::addRoute(GET, "/{file}", test_target);
}
