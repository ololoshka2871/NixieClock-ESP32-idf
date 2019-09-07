#ifndef _ID1_PARRALEL_POLICU_H_
#define _ID1_PARRALEL_POLICU_H_

#include <vector>
#include <memory>

namespace DynamicIndication
{
template <typename T>
struct ic7414ParralelPolicy
{
    using bus_type = T;
    using data_type = typename bus_type::data_type;

    static const uint32_t bits_pre_channel = 4;

    ic7414ParralelPolicy() : channels_pre_data_bus(0), group_count(0) {}

    ic7414ParralelPolicy(std::shared_ptr<bus_type> &&_dataBus, const uint32_t group_count) : databus(std::move(_dataBus)),
                                                                                             channels_pre_data_bus(this->databus->width() / bits_pre_channel),
                                                                                             group_count(group_count) {}

    ic7414ParralelPolicy(const ic7414ParralelPolicy &) = delete;

    void setData(const std::vector<data_type> &dataSrc, uint8_t group)
    {
        uint32_t bus_val = 0;
        for (uint ch = 0; ch < channels_pre_data_bus; ++ch)
        {
            auto d = dataSrc.at(group + ch * group_count);
            bus_val |= d << (bits_pre_channel * ch);
        }
        databus->setData(bus_val);
    }

    bool configured() const { return group_count > 0 && databus != nullptr && databus->width() > 0; }

private:
    std::shared_ptr<bus_type> databus;
    const uint32_t channels_pre_data_bus;
    const uint32_t group_count;
};
} // namespace DynamicIndication

#endif /* _ID1_PARRALEL_POLICU_H_ */
