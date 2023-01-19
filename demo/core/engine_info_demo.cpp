#include "tz/tz.hpp"
#include "hdk/debug.hpp"
#include <cstdio>

int main()
{
	constexpr tz::EngineInfo info = tz::info();
	hdk::report("Engine Info: %s", info.to_string().c_str());
}