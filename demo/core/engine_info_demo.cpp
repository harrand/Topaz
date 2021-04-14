#include "core/tz.hpp"
#include <cstdio>

int main()
{
    constexpr tz::EngineInfo info = tz::info();
    std::printf("%s", info.to_string().c_str());
}