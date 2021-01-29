#include <type_traits>

namespace tz
{
    template<typename T>
    T rand()
    {
        return detail::device()();
    }

    template<typename T>
    T range(T min, T max)
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