#ifndef _DI_CONTROLLER_H_
#define _DI_CONTROLLER_H_

#include <initializer_list>

namespace DynamicIndication {

template <typename TDataPolicy, typename TSelectorPolicy, typename TEncoder,
          typename TEffector>
struct Controller {
  using DataPolicy_t = TDataPolicy;
  using SelectorPolicy_t = TSelectorPolicy;
  using Encoder_t = TEncoder;
  using Effector_t = TEffector;

  Controller() = default;

  DataPolicy_t &getDataPolicy() { return datapolicy; }
  SelectorPolicy_t &getSelectorPolicy() { return selectorPolicy; }
  Encoder_t &getEncoder() { return encoder; }
  Effector_t &getEffector() { return effector; }

private:
  DataPolicy_t datapolicy;
  SelectorPolicy_t selectorPolicy;
  Encoder_t encoder;
  Effector_t effector;
};

} // namespace DynamicIndication

#endif /* _DI_CONTROLLER_H_ */
