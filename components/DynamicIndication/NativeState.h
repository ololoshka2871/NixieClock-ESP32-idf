#ifndef _NATIVE_STATE_H_
#define _NATIVE_STATE_H_

#include <vector>
#include <memory>
#include <atomic>

namespace DynamicIndication
{
template <typename S, typename D>
struct NativeState
{
	using selector_t = S;
	using display_policy_t = D;
	using DataBus_t = typename display_policy_t::bus_type;
	using SelectorBus_t = typename selector_t::bus_type;

	NativeState(std::shared_ptr<DataBus_t> &&dataBus,
				std::shared_ptr<SelectorBus_t> &&selectorBus,
				const int indicators_count)
		: selector(std::move(selectorBus)),
		  displayPolicy(std::move(dataBus), selector.group_count()),
		  m_data(indicators_count),
		  update_interval_us(10000),
		  indicators_count(indicators_count)
	{
		register_timer();
	}

	~NativeState()
	{
		if (timer_idx < 0)
		{
			return;
		}
		setEnabled(false);
		unregister_timer();
	}

	bool setEnabled(bool enabled = true)
	{
		bool res = enabled
					   ? EnableTimer()
					   : DisableTimer();

		selector.setEnabled(enabled);
		selector.next_element();
		return res;
	}

	bool isEnabled() const { return selector.isEnabled(); }

	void setUpdateInterval(uint32_t interval_us)
	{
		update_interval_us = interval_us;
		if (isEnabled())
		{
			EnableTimer();
		}
	}

	uint32_t getUpdateInterval() const { return update_interval_us; }

	void setData(CLR_RT_TypedArray_UINT32 &data)
	{
		size_t size = std::min(data.GetSize(), m_data.size());
		for (size_t i = 0; i < size; ++i)
		{
			m_data[i] = (typename DataBus_t::data_type)data.GetValue(i);
		}
	}

	// check if config ok
	bool configured() const
	{
		return selector.configured() && displayPolicy.configured() &&
			   (timer_idx >= 0);
	}

private:
	selector_t selector;
	display_policy_t displayPolicy;
	std::vector<typename DataBus_t::data_type> m_data;
	uint32_t update_interval_us;
	int timer_idx;
	uint8_t indicators_count;

	bool EnableTimer()
	{
		if ((timer_idx < 0) ||
			(isEnabled() && !DisableTimer()))
		{
			return false;
		}

		return esp_timer_start_periodic(
				   hrtimers[timer_idx],
				   update_interval_us) == ESP_OK;
	}

	bool DisableTimer()
	{
		if (timer_idx < 0)
		{
			return false;
		}

		auto res = esp_timer_stop(hrtimers[timer_idx]) == ESP_OK;

		return res;
	}

	void NextGroup() { displayPolicy.setData(m_data, selector.next_element()); }

	void register_timer()
	{
		timer_idx = FindNextTimerIndex();
		if (timer_idx < 0)
		{
			return;
		}

		esp_timer_handle_t newTimerHandle;
		esp_timer_create_args_t timer_args{timer_cb, this, ESP_TIMER_TASK,
										   "Dynamic_updater_timer"};
		esp_err_t err = esp_timer_create(&timer_args, &newTimerHandle);
		if (err != ESP_OK)
		{
			timer_idx = -1;
			return;
		}

		hrtimers[timer_idx] = newTimerHandle;
	}

	void unregister_timer()
	{
		esp_timer_delete(hrtimers[timer_idx]);
		hrtimers[timer_idx] = 0;
	}

	static void timer_cb(void *arg)
	{
		auto _this = static_cast<NativeState *>(arg);
		_this->NextGroup();
	}

	static int FindNextTimerIndex()
	{
		for (int index = 0; index < MAX_HRTIMERS; index++)
		{
			if (hrtimers[index] == 0)
				return index;
		}

		return -1;
	}
};
} // namespace DynamicIndication

#endif //_NATIVE_STATE_H_
