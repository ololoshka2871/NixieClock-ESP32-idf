#include <chrono>
#include <functional>
#include <iostream>
#include <sstream>

#include <RTC.h>

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

#include "GUI/GUI.h"

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

enum class colorComponenta : uint8_t { RED = 0, GREEN = 1, BLUE = 2 };

template <colorComponenta componenta> json getClorComponenta() {
  return json::string_t{to_string(
      GUI::getClockBGColorComponenta(static_cast<uint8_t>(componenta)))};
}

template <colorComponenta componenta>
void setClorComponenta(const json &value) {
  if (value.is_string()) {
    uint32_t c;
    auto s = value.get<std::string>();
    auto r = sscanf(s.c_str(), "%u", &c);
    if (r == 1) {
      GUI::setClockBGColorComponenta(static_cast<uint8_t>(componenta),
                                     static_cast<uint8_t>(c));
    } else {
      ESP_LOGE(LOG_TAG, "Failed to parce color componenta: %s", s.c_str());
    }
  }
}

/******************************************************************************/

struct VirtuinoJsonServerThread : public support::cancellableThread {
  VirtuinoJsonServerThread(uint16_t port)
      : support::cancellableThread{"Virtuino", configMINIMAL_STACK_SIZE + 4096,
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

  static inline const std::vector<valuesAccessors> value_router {
    // V0 - clock
    {[]() {
       int64_t miliseconds =
           std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::system_clock::now().time_since_epoch())
               .count();
       return json::string_t{to_string(miliseconds)};
     },
     [](const json &newval) {
       if (newval.is_string()) {
         uint64_t t_ms;
         auto s = newval.get<std::string>();
         auto r = sscanf(s.c_str(), "%lld", &t_ms);
         if (r == 1) {
           timeval tv{static_cast<time_t>(t_ms / 1000),
                      static_cast<__suseconds_t>((t_ms % 1000) * 1000)};
           RTCManager::instance()->setupRTC(tv);
         } else {
           ESP_LOGE(LOG_TAG, "parsing %s, res = %d", s.c_str(), r);
         }
       }
     }},
        // vo color
        {[]() {
           union {
             int32_t _signed;
             uint32_t _unsigned;
           } v;
           v._unsigned = GUI::getClockBGColor();
           v._unsigned = v._unsigned | (0xFF << 24);
           return json::string_t{to_string(v._signed)};
         },
         [](const json &value) {
           if (value.is_string()) {
             union {
               int32_t _signed;
               uint32_t _unsigned;
             } v;
             auto s = value.get<std::string>();
             auto r = sscanf(s.c_str(), "%d", &v._signed);
             if (r == 1) {
               // see https://stackoverflow.com/a/38570206
               v._unsigned = v._unsigned & ((1 << 24) - 1);
               GUI::setClockBGColor(v._unsigned);
             } else {
               ESP_LOGE(LOG_TAG, "failed to get int from %s", s.c_str());
             }
           }
         }},
#if 0
      // V1 - R
      {getClorComponenta<colorComponenta::RED>,
       setClorComponenta<colorComponenta::RED>},
      // V2 - G
      {getClorComponenta<colorComponenta::GREEN>,
       setClorComponenta<colorComponenta::GREEN>},
      // V3 - B
      {getClorComponenta<colorComponenta::BLUE>,
       setClorComponenta<colorComponenta::BLUE>},
#endif
  };

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
#if 0
    ESP_LOGI(LOG_TAG, "\n<==JSON:");
    std::cout << result << std::endl;
#endif
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
    std::cout << inputJson << std::endl;

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

    socketstream << reportVars(inputJson) << std::endl;
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
