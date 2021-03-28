#include "core/time.hpp"
#include <chrono>

namespace tz
{
    TimePoint time_millis()
    {
        using namespace std::chrono;
        milliseconds ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
        return ms.count();  
    }
}