#include <functional>
#include <iostream>
#include <sstream>

#include <esp_log.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <lwip/api.h>
#include <lwip/err.h>

#include "String_format.h"

#include "nlohmann/json.hpp"

//#include "NetAdapter.h"
#include "mDNSServer.h"

#include "basic_socketbuf.hh"
#include "basic_socketstream.hh"
#include "servers/lwip_socket_traits.h"

#include <cancellableThread.h>

#include "VirtuinoJsonServer.h"

using json = nlohmann::json;

static constexpr char LOG_TAG[] = "VirtuinoSE";
static constexpr char api_key[] = "1234";

static constexpr uint8_t V_MAX = 255;

std::unique_ptr<support::cancellableThread> thread;

/******************************************************************************/

using socketbuf = swoope::basic_socketbuf<swoope::lwip_socket_traits>;
using socketstream = swoope::basic_socketstream<swoope::lwip_socket_traits>;

/******************************************************************************/

static bool varify_api_key(json &input) {
  auto it = input.find("key");
  if (it != input.end()) {
    auto value = *it->cbegin();
    if (value.is_string() && value == api_key) {
      return true;
    } else {
      ESP_LOGE(LOG_TAG, "Input API key invalid: %s != %s",
               value.get<std::string>().c_str(), api_key);
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
        port(port) {}

protected:
  static constexpr char name[] = "Virtuino SE server";
  static constexpr char proto[] = "_tcp";

  struct valuesAccessors {
    using getter_t = std::function<json()>;
    using setter_t = std::function<void(const json &)>;

    valuesAccessors(getter_t &&getter, setter_t &&setter)
        : getter{std::move(getter)}, setter{std::move(setter)} {}

    const getter_t getter;
    const setter_t setter;
  };

  static inline const std::vector<valuesAccessors> value_router{
      {[]() { return json::number_integer_t(0); }, nullptr}};

  json parceInput(std::istream &input) {
    json result;
    input >> result;
    return result;
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
    mDNSServer::instance().removeService(name, proto);
    cancellableThread::Cleanup();
  }

  static void send_wrong_key(socketstream &socketstream) {
    json result;
    result["status"] = -1;
    result["message"] = "Wrong Key";

    socketstream << result;
  }

  static void send_check_responce(socketstream &socketstream) {
    json result;
    result["status"] = 2;
    result["message"] = "Hello Virtuino";

    socketstream << result;
  }

  static void store_error(json &json, uint32_t val_num) {
    json["message"] = format("No V%u, present", val_num);
  }

  json reportVars(json &input) {
    using namespace std::string_literals;

    json result;
    int status = 1;

    for (uint32_t i = 0; i <= V_MAX; ++i) {
      auto key = format("V%d_", i);

      const auto it = input.find(key);
      if (it != input.end()) {
        if (i >= std::size(value_router)) {
          store_error(result, i);
          status = -1;
          break;
        }

        const auto _itv = it->find("value");
        if (_itv != it->end()) {
          const auto _v = *_itv->cbegin();
          if (_v.is_string()) {
            if (_v.get<std::string>() == "?"s) {
              const auto _res = value_router[i].getter();
              result[key] = json{{"value", _res}};
            } else {
              auto &setter = value_router[i].setter;
              if (setter) {
                setter(_v);
              } else {
                status = -1;
                result["message"] = format("value V%u not writable", i);
              }
            }
          }
        }
      }
    }

    result["status"] = to_string(status);

    ESP_LOGI(LOG_TAG, "\n<==JSON:");
    std::cout << result;

    return result;
  }

  void process_clientConnection(socketstream &socketstream) {
    using namespace std::string_literals;

    auto inputJson = parceInput(socketstream);
    if (inputJson.empty()) {
      ESP_LOGE(LOG_TAG, "Failed to parce input json");
      return;
    }
    if (!varify_api_key(inputJson)) {
      send_wrong_key(socketstream);
      return;
    }

    ESP_LOGI(LOG_TAG, "\n==>JSON:");
    std::cout << inputJson;

    {
      auto status = inputJson.find("status");
      if (status != inputJson.end() && status->cbegin()->is_string()) {
        auto s = status->cbegin()->get<std::string>();
        if (s == "0"s) {
          send_check_responce(socketstream);
        } else {
          ESP_LOGE(LOG_TAG, "Unknown status: %s", s.c_str());
          return;
        }
      }
    }

    socketstream << reportVars(inputJson);
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
