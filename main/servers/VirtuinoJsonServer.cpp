#include <iostream>
#include <sstream>

#include <esp_log.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <lwip/api.h>
#include <lwip/err.h>

#include "String_format.h"

#include "my-jsl-parser.h"

//#include "NetAdapter.h"
#include "mDNSServer.h"

#include "basic_socketbuf.hh"
#include "basic_socketstream.hh"
#include "servers/lwip_socket_traits.h"

#include <cancellableThread.h>

#include "VirtuinoJsonServer.h"

static constexpr char LOG_TAG[] = "VirtuinoSE";

static constexpr char api_key[] = "1234";

std::unique_ptr<support::cancellableThread> thread;

/******************************************************************************/

using socketbuf = swoope::basic_socketbuf<swoope::lwip_socket_traits>;
using socketstream = swoope::basic_socketstream<swoope::lwip_socket_traits>;

/******************************************************************************/

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
    ESP_LOGE(LOG_TAG, "invalid request, key not found");
    return false;
  }
}

/******************************************************************************/

struct VirtuinoJsonServerThread : public support::cancellableThread {
  VirtuinoJsonServerThread(uint16_t port)
      : support::cancellableThread{"Virtuino", configMINIMAL_STACK_SIZE + 2048,
                                   10},
        port(port) {
    reset_json_parcer();
  }

protected:
  static constexpr char name[] = "Virtuino SE server";
  static constexpr char proto[] = "_tcp";

  void reset_json_parcer() { jsl_data_pool::init(100, 20, 20); }

  jsl_data_dict *parceInput(std::istream &input) {
    reset_json_parcer();
    my_jsl_parser parser(input);
    return parser.parse_no_seek();
  }

  void Run() override {
    mDNSServer::instance().addService(name, "_virtuino", proto, port,
                                      {{"board", "esp32"}});

    socketstream server;
    server.open(to_string(port), 2);
    ESP_LOGI(LOG_TAG, "Server listening on %d/tcp", port);

    while (!testCancel()) {

      socketstream client;

      server.accept(client);
      if (client.is_open()) {
        process_clientConnection(client);
        client.shutdown(std::ios_base::out);
        client.close();
      }

      vTaskDelay(1);
    }

    server.close();
  }

  void Cleanup() override {
    jsl_data_pool::init(0, 0, 0);
    mDNSServer::instance().removeService(name, proto);
    cancellableThread::Cleanup();
  }

  void send_wrong_key(socketstream &socketstream) {
    socketstream << "{ \"status\":-1, \"message\": \"Wrong Key\"}" << std::endl;
  }

  void process_clientConnection(socketstream &socketstream) {
    auto inputJson = parceInput(socketstream);
    if (!inputJson) {
      ESP_LOGE(LOG_TAG, "Failed to parce input json");
      return;
    }
    if (!varify_api_key(inputJson)) {
      send_wrong_key(socketstream);
      return;
    }

    std::string status;
    if (inputJson->get("status", status)) {
      if (status == "0") {
        jsl_data_dict result;
        jsl_data_scal status_code{2};
        jsl_data_scal msg{"Hello Virtuino"};
        result.set_prop("status", status_code);
        result.set_prop("message", msg);
        result.encode(std::cout, true);
        result.encode(socketstream, true);
      } else {
        ESP_LOGE(LOG_TAG, "Unknown status: %s", status.c_str());
        return;
      }
    }

    /*
    if (status == "1") {
      jsl_data_dict vardata;
      auto t = &vardata;
      if (!inputJson->get("V0_", t)) {
        netconn_write(&clientConnection, wrong_key, sizeof(wrong_key),
                      NETCONN_NOCOPY);
      }
    }*/
  }

  const uint16_t port;
};

void VirtuinoJsonServer::start(uint16_t port) {
  if (!thread) {
    thread = std::make_unique<VirtuinoJsonServerThread>(port);
  }
  thread->Restart();
}

void VirtuinoJsonServer::stop() {
  assert(thread);
  thread->cancel().join();
  thread.reset();
}
