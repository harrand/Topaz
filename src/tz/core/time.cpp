#include "tz/core/time.hpp"
#include <chrono>

namespace tz
{
	Duration system_time()
	{
		auto system = std::chrono::system_clock::now().time_since_epoch();
		return {static_cast<unsigned long long>(std::chrono::duration_cast<std::chrono::nanoseconds>(system).count())};
	}

	Delay::Delay(Duration delay_length):
	begin_systime(system_time()),
	delay_length(delay_length)
	{}

	bool Delay::done() const
	{
		return (this->begin_systime + this->delay_length) <= system_time();
	}

	void Delay::reset()
	{
		this->begin_systime = system_time();
	}
}
