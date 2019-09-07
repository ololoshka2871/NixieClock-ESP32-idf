#ifndef _DI_CONTROLLER_H_
#define _DI_CONTROLLER_H_

#include <initializer_list>

namespace DynamicIndication {

template<typename TDataPolicy, typename TSelectorPolicy, typename TEncoder>
struct Controller {
    using DataPolicy_t = TDataPolicy;
    using SelectorPolicy_t = TSelectorPolicy;
    using Encoder_t = TEncoder;

    Controller() = default;

    DataPolicy_t& getDataPolicy() { return datapolicy; }
    SelectorPolicy_t& getSelectorPolicy() { return selectorPolicy; }
    Encoder_t& getEncoder() { return encoder; }

private:
    DataPolicy_t datapolicy;
    SelectorPolicy_t selectorPolicy;
    Encoder_t encoder;
};

}

#endif /* _DI_CONTROLLER_H_ */
