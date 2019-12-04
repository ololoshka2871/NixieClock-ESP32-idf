#include <iostream>
#include <sstream>

#include <esp_log.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <lwip/api.h>

#include <json/jsl-parser.h>

#include "NetAdapter.h"
#include "mDNSServer.h"

#include "VirtuinoJsonServer.h"

static constexpr char _ssid[] = "DIR-300NRU";
static constexpr char _pass[] = "*Bu@#?Kz,u.{-guG";

static constexpr char LOG_TAG[] = "Virtuino SE JSON";
static constexpr char SSID[] = "NIXIE-ESP32";
static constexpr char PASS[] = "iddqdidkfa";

static constexpr char api_key[] = "1234";

/**********************************************************************************************************************/

static jsl_data_dict *parceInput(std::istream &input) {
  jsl_parser parser(input);
  return parser.parse();
}

static std::stringstream readRequest(netconn &conn) {
  netbuf *inbuf = nullptr;
  auto ret = netconn_recv(&conn, &inbuf);
  if (ret != ERR_OK)
    return std::stringstream();

  std::stringstream stream;
  do {
    char *bufptr;
    u16_t buflen;
    netbuf_data(inbuf, (void **)&bufptr, &buflen);
    stream.write(bufptr, buflen);
  } while (netbuf_next(inbuf) == 0);

  netbuf_delete(inbuf);
  return stream;
}

static bool varify_api_key(jsl_data_dict *input) {
  std::string key;
  if (input->get("key", key)) {
    if (key == api_key) {
      return true;
    } else {
      ESP_LOGE(LOG_TAG, "Input API key invalid: %s != %s", key.c_str(),
               api_key);
      return false;
    }
  } else {
    ESP_LOGE(LOG_TAG, "invalid request, ky not found");
    return false;
  }
}

void VirtuinoJsonServer::begin() {
  createSoftAP();
  createMdnsRecord();
}

void VirtuinoJsonServer::start(uint16_t port) {
  // не через std::thread, чтобы задать размер стека
  xTaskCreate(&VirtuinoJsonServer::Run, "VirtuinoJsonServer", 4096,
              reinterpret_cast<void *>(port), 10, nullptr);
}

void VirtuinoJsonServer::createMdnsRecord() {
  mDNSServer::instance().addService("Virtuino SE server", "_virtuino", "_tcp",
                                    8191, {{"board", "esp32"}});
}

void VirtuinoJsonServer::createSoftAP() {
  if (_ssid) {
    if (NetAdapter::instance().tryConnect(_ssid, _pass) == ESP_OK) {
      return;
    }
  }
  NetAdapter::instance().createSoftap(SSID, PASS);
}

void VirtuinoJsonServer::Run(void *ctx) {
  const uint16_t port = reinterpret_cast<uint32_t>(ctx);
  err_t ret;

  netconn *conn;
  netconn *newconn;

  conn = netconn_new(NETCONN_TCP);
  netconn_bind(conn, IP_ADDR_ANY, port);
  netconn_listen(conn);

  ESP_LOGI(LOG_TAG, "Server listening on port %d", port);

  netconn_set_nonblocking(conn, 1);

  do {
    ret = netconn_accept(conn, &newconn);
    if (ret == ERR_OK && newconn != nullptr) {
      process_clientConnection(*newconn);
      netconn_delete(newconn);
    }

    vTaskDelay(1);
  } while (ret != ERR_MEM && ret != ERR_BUF && ret != ERR_VAL &&
           ret != ERR_IF && ret != ERR_ARG);

  netconn_close(conn);
  netconn_delete(conn);

  vTaskDelete(nullptr);
}

void VirtuinoJsonServer::process_clientConnection(netconn &clientConnection) {
  jsl_data_pool::init(100, 20, 20);

  auto istream = readRequest(clientConnection);
  auto inputJson = parceInput(istream);
  if (!inputJson) {
    ESP_LOGE(LOG_TAG, "Failed to parce input json:\n%s", istream.str().c_str());
    return;
  }
  if (!varify_api_key(inputJson)) {
    return;
  }

  inputJson->encode(std::cout, true);

  std::string status;
  if (!inputJson->get("status", status)) {
    static constexpr char err[] =
        "{ \"status\":-1, \"message\": \"Wrong Key\"}";
    netconn_write(&clientConnection, err, sizeof(err), NETCONN_NOCOPY);
    return;
  }
  if (status == "0") {
    static constexpr char ok[] = "{ \"status\":\"2\", \"message\": \"Hello "
                                 "Virtuino\"}";
    netconn_write(&clientConnection, ok, sizeof(ok), NETCONN_NOCOPY);
  }

  jsl_data_pool::init(0, 0, 0);
}
