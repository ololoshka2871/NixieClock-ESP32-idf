#ifndef _DATABUS_H_
#define _DATABUS_H_

#include <vector>
#include <type_traits>
#include <algorithm>

#include <driver/gpio.h>

namespace DynamicIndication
{
template <typename Tv = uint32_t>
struct DataBus
{
    using data_type = Tv;

    static_assert(!std::numeric_limits<data_type>::is_signed, "Tv mast be unsigned");
    static_assert(std::numeric_limits<data_type>::is_integer, "Tv mast be integer");

    DataBus() {}

    DataBus(std::initializer_list<int> pins, bool invert = false) : pinsNumbers(pins), value(), invert(invert)
    {
        auto it = pinsNumbers.begin();
        std::size_t i = 0;
        for (; it != pinsNumbers.end(); ++it, ++i)
        {
            /*
            auto pn = static_cast<gpio_num_t>(pins.GetValue(i));
            *it = pn;
            gpio_reset_pin(pn);
            gpio_set_direction(pn, GPIO_MODE_OUTPUT);
            gpio_set_pull_mode(pn, GPIO_FLOATING);
            */
        }
        setData(invert ? std::numeric_limits<data_type>::max() : 0);
    }

    ~DataBus()
    {
        std::for_each(pinsNumbers.cbegin(), pinsNumbers.cend(), [](gpio_num_t pn) {
            gpio_reset_pin(pn);
        });
    }

    void setData(data_type data)
    {
        value = data;
        for (std::size_t i = 0; i < pinsNumbers.size(); ++i)
        {
            auto pn = pinsNumbers[i];
            gpio_set_level(pn, (((data & 1u << i) != 0) ^ invert));
        }
    }

    data_type getData() const
    {
        return value;
    }

    int width() const { return pinsNumbers.size(); }

    void setInverted(bool inverted)
    {
        if (inverted != invert)
        {
            auto oldata = getData();
            invert = inverted;
            setData(oldata);
        }
    }

    bool isInverted() const { return invert; }

private:
    std::vector<gpio_num_t> pinsNumbers;
    data_type value;
    bool invert;
};
} // namespace DynamicIndication

#endif //_DATABUS_H_
