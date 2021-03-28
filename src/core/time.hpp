#ifndef TOPAZ_CORE_TIME_HPP
#define TOPAZ_CORE_TIME_HPP

namespace tz
{
    using TimePoint = unsigned long long;
    /**
     * Retrieve the number of milliseconds passed since epoch.
     */
    TimePoint time_millis();
}

#endif // TOPAZ_CORE_TIME_HPP