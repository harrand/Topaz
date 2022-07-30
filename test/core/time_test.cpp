#include "tz/core/time.hpp"
#include "tz/core/assert.hpp"
#include <thread>

void test_waits()
{
	using namespace tz::literals;
	using namespace std::chrono_literals;
	tz::Delay d{10_ms};
	tz_assert(!d.done(), "Delay of 10 millis was apparantly done instantly.");
	std::this_thread::sleep_for(11ms);
	tz_assert(d.done(), "Delay of 10 millis was not done after a this_thread_sleep_for 11ms");

	d.reset();
	tz_assert(!d.done(), "Delay of 10 millis was apparantly done instantly.");
	tz_assert(!d, "Delay of 10 millis was apparantly done instantly.");
	std::this_thread::sleep_for(11ms);
	tz_assert(d.done(), "Delay of 10 millis was not done after a this_thread_sleep_for 11ms");
	tz_assert(d, "Delay of 10 millis was not done after a this_thread_sleep_for 10ms");
}

int main()
{
	using namespace tz::literals;
	constexpr tz::Duration s = 1_s;
	static_assert(s == 1000_ms);
	static_assert(s == 1000000_us);
	static_assert(s == 1000000000_ns);
	static_assert(s.seconds<int>() == 1);
	static_assert(s.seconds<float>() == 1.0f);
	static_assert(s.seconds<unsigned int>() == (1000_ms).seconds<float>());
	static_assert(s > 500_ms && s < 2_s);
	test_waits();
}
