#include "core/tz.hpp"

int main()
{
    std::string enginfo = tz::info().to_string();
    std::printf("EngineInfo: %s", enginfo.c_str());
    return 0;
}