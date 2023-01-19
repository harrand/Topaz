#include "tz/tz.hpp"
#include "tz/core/debug.hpp"
#include <cstdio>

int main()
{
	constexpr tz::EngineInfo info = tz::info();
	tz::report("Engine Info: %s", info.to_string().c_str());
}