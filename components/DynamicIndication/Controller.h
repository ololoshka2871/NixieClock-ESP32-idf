#ifndef _DI_CONTROLLER_H_
#define _DI_CONTROLLER_H_

#include <cassert>
#include <memory>

#include "Effects/NoEffect.h"

#include <esp_timer.h>

namespace DynamicIndication {

template <typename TDataPolicy, typename TSelectorPolicy, typename TEncoder,
          typename TeffetorPtr =
              std::unique_ptr<Effects::NoEffect<typename TEncoder::Output_t>>>
struct Controller {
  using this_type =
      Controller<TDataPolicy, TSelectorPolicy, TEncoder, TeffetorPtr>;
  using DataPolicy_t = TDataPolicy;
  using SelectorPolicy_t = TSelectorPolicy;
  using Encoder_t = TEncoder;
  using framebufferItem_t = typename Encoder_t::Output_t;
  using Effector_t = Effects::IEffect<framebufferItem_t>;

  static constexpr uint64_t defaultUpdateInterval_us = 10000;

  Controller(DataPolicy_t &datapolicy, SelectorPolicy_t &selectorPolicy,
             Encoder_t &encoder)
      : datapolicy(datapolicy), selectorPolicy(selectorPolicy),
        encoder(encoder), effector(new Effects::NoEffect<framebufferItem_t>),
        update_interval_us(defaultUpdateInterval_us) {
    effector.initBuffers((datapolicy.framebufferSize()));
    register_timer();
  }

  Controller(DataPolicy_t &datapolicy, SelectorPolicy_t &selectorPolicy,
             Encoder_t &encoder, TeffetorPtr pEffector)
      : datapolicy(datapolicy), selectorPolicy(selectorPolicy),
        encoder(encoder), effector(pEffector),
        update_interval_us(defaultUpdateInterval_us) {
    effector->initBuffers(datapolicy.framebufferSize());
    register_timer();
  }

  Controller(const Controller &) = delete;

  ~Controller() {
    setEnabled(false);
    unregister_timer();
  }

  DataPolicy_t &getDataPolicy() { return datapolicy; }
  SelectorPolicy_t &getSelectorPolicy() { return selectorPolicy; }
  Encoder_t &getEncoder() { return encoder; }
  Effector_t &getEffector() { return *effector; }

  this_type &setText(const std::string &text) {
    effector->setDestinationData(encoder.encode(text));
    return *this;
  }

  this_type &setEnabled(bool enabled = true) {
    if (enabled) {
      EnableTimer();
    } else {
      DisableTimer();
    }

    selectorPolicy.setEnabled(enabled);
    selectorPolicy.next_element();
    return *this;
  }

  bool isEnabled() const { return selectorPolicy.isEnabled(); }

  this_type &setUpdateInterval(uint32_t interval_us) {
    update_interval_us = interval_us;
    if (isEnabled()) {
      EnableTimer();
    }
    return *this;
  }

  uint32_t getUpdateInterval() const { return update_interval_us; }

private:
  DataPolicy_t &datapolicy;
  SelectorPolicy_t &selectorPolicy;
  Encoder_t &encoder;
  TeffetorPtr effector;
  uint64_t update_interval_us;

  esp_timer_handle_t timerHandle;

  bool EnableTimer() {
    if (!timerHandle || (isEnabled() && !DisableTimer())) {
      return false;
    }

    return esp_timer_start_periodic(timerHandle, update_interval_us) == ESP_OK;
  }

  bool DisableTimer() {
    if (!timerHandle) {
      return false;
    }

    return esp_timer_stop(timerHandle) == ESP_OK;
  }

  esp_err_t register_timer() {
    const esp_timer_create_args_t timer_args{
        [](void *arg) {
          auto _this = static_cast<Controller *>(arg);
          _this->NextGroup();
        },
        this, ESP_TIMER_TASK, "Dynamic_updater_timer"};
    return esp_timer_create(&timer_args, &timerHandle);
  }

  void unregister_timer() {
    if (timerHandle) {
      esp_timer_delete(timerHandle);
    }
  }

  void NextGroup() {
    effector->frameOp([this](auto frame) {
      datapolicy.setData(frame, selectorPolicy.next_element());
    });

    if (selectorPolicy.isFirstElement()) {
      effector->nextFrame();
    }
  }
};

} // namespace DynamicIndication

#endif /* _DI_CONTROLLER_H_ */
