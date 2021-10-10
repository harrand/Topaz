#include "core/tz.hpp"
#include "core/report.hpp"
#include <cstdio>

int main()
{
	constexpr tz::EngineInfo info = tz::info();
	tz_report("Engine Info: %s", info.to_string().c_str());
}