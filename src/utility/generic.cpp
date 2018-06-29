#include "generic.hpp"

namespace tz::utility::generic
{
    namespace literals
    {
        long double operator ""_lb(long double mass)
        {
            return mass * 0.45359237;
        }

        long double operator""_st(long double mass)
        {
            using namespace tz::utility::generic::literals;
            return operator""_lb(mass * 14.0);
        }

        inline long double operator""_deg(long double angle)
        {
            return angle * tz::utility::numeric::consts::pi / 180.0;
        }
    }
}
