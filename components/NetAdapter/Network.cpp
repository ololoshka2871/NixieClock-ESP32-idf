#include <esp_log.h>
#include <esp_wifi.h>
#include <nvs_flash.h>

#include <freertos/event_groups.h>

#include "STA.h"
#include "SoftAP.h"

#include "Network.h"

using namespace nw;

static std::shared_ptr<IAdapter> ap;
static std::shared_ptr<IAdapter> sta;
static EventGroupHandle_t event_group;

static void initStorage() { ESP_ERROR_CHECK(nvs_flash_init()); }

static void initTcpip() {
  tcpip_adapter_init();
  /*
  IP4_ADDR(&ApIPInfo.ip, 192, 168, 35, 1);
  IP4_ADDR(&ApIPInfo.gw, 192, 168, 35, 1);
  IP4_ADDR(&ApIPInfo.netmask, 255, 255, 255, 0);
  */
}

static void initEventLoop() {
  // esp::DefaultEventLoop::instance();
  event_group = xEventGroupCreate();
}

static void resetWiFi() {
  wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));
  ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
}

static void init_hw_if_disabled() {
  if (!ap && !sta) {
    initStorage();
    initTcpip();
    initEventLoop();
    resetWiFi();
  }
}

static void free_hw_if_idle() {
  if (!ap && !sta) {
    esp_wifi_deinit();
  }
}

template <typename T>
static void make_adapter(std::shared_ptr<IAdapter> &dest) {
  struct deleter {
    const char *LOG_TAG = "nw::make_adapter::deleter";

    deleter(std::shared_ptr<IAdapter> &adapter) : adapter(adapter) {}

    deleter(const deleter &oher) : adapter(oher.adapter) {
      ESP_LOGI(LOG_TAG, "Copy constructor");
    }
    void operator=(const deleter &oher) = delete;

    deleter(deleter &&oher) : adapter(oher.adapter) {
      ESP_LOGI(LOG_TAG, "Move constructor");
    }

    void operator=(const deleter &&oher) = delete;

    void operator()(T *ptr) {
      delete ptr;

      ESP_LOGI(LOG_TAG, "Adapters in use: %ld", adapter.use_count());
      if (adapter.use_count() == 1) {
        ESP_LOGI(LOG_TAG, "Destroing network adapter");
        adapter.reset();
        free_hw_if_idle();
      }
    }

  private:
    std::shared_ptr<IAdapter> &adapter;
  };

  if (!dest) {
    init_hw_if_disabled();

    dest = std::shared_ptr<T>(new T, deleter(dest));
    ESP_LOGI("nw::creator", "Network adapter created");
  }
}

std::shared_ptr<IAdapter> nw::getSoftAP() {
  make_adapter<SoftAP>(ap);
  return ap;
}

std::shared_ptr<IAdapter> nw::getSTA() {
  make_adapter<STA>(sta);
  return sta;
}
