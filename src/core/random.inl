#include <type_traits>

namespace tz
{
    template<tz::Number T>
    T rand()
    {
        return detail::device()();
    }

    template<tz::Number T>
    T range(tz::Number auto min, tz::Number auto max)
    {
        if constexpr(std::is_integral_v<T>)
        {
            return std::uniform_int_distribution<T>{min, max}(detail::device());
        }
        else
        {
            // Use a real distribution.
            return std::uniform_real_distribution<T>{min, max}(detail::device());
        }
    }
}