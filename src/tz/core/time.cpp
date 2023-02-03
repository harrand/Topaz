#include "tz/core/time.hpp"
#include <chrono>

namespace tz
{
	duration system_time()
	{
		auto system = std::chrono::system_clock::now().time_since_epoch();
		return {static_cast<unsigned long long>(std::chrono::duration_cast<std::chrono::nanoseconds>(system).count())};
	}

	delay::delay(duration delay_length):
	begin_systime(system_time()),
	delay_length(delay_length)
	{}

	bool delay::done() const
	{
		return this->elapsed() >= this->delay_length;
	}

	duration delay::elapsed() const
	{
		return this->begin_systime - system_time();
	}

	void delay::reset()
	{
		this->begin_systime = system_time();
	}
}
