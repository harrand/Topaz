#include "tz/core/time.hpp"
#include "tz/core/debug.hpp"
#include <thread>

int main()
{
	using namespace tz::literals;
	constexpr tz::duration s = 1_s;
	static_assert(s == 1000_ms);
	static_assert(s == 1000000_us);
	static_assert(s == 1000000000_ns);
	static_assert(s.seconds<int>() == 1);
	static_assert(s.seconds<float>() == 1.0f);
	static_assert(s.seconds<unsigned int>() == (1000_ms).seconds<float>());
	static_assert(s > 500_ms && s < 2_s);
}
