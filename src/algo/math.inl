#include <cmath>

namespace tz::algo
{
    template<typename T>
    T schmitt(T lo, T hi, T val, SchmittBound bound)
    {
        T hdist = std::abs(hi - val);
        T ldist = std::abs(val - lo);

        if(bound == SchmittBound::Lower)
        {
            if(hdist >= ldist)
            {
                return lo;
            }
            else
            {
                return hi;
            }
        }
        else if(bound == SchmittBound::Higher)
        {
            if(hdist <= ldist)
            {
                return hi;
            }
            else
            {
                return lo;
            }
        }
        else
        {
            topaz_assert(false, "tz::algo::schmitt(", lo, ", ", hi, ", ...): SchmittBound unrecognised (not lo nor hi).");
            return T{};
        }
    }

    template<typename T, typename F>
    T schmittf(T lo, T hi, F val, SchmittBound bound)
    {
        F hdist = std::abs(static_cast<F>(hi) - val);
        F ldist = std::abs(val - static_cast<F>(lo));

        if(bound == SchmittBound::Lower)
        {
            if(hdist >= ldist)
            {
                return lo;
            }
            else
            {
                return hi;
            }
        }
        else if(bound == SchmittBound::Higher)
        {
            if(hdist <= ldist)
            {
                return hi;
            }
            else
            {
                return lo;
            }
        }
        else
        {
            topaz_assert(false, "tz::algo::schmittf(", lo, ", ", hi, ", ...): SchmittBound unrecognised (not lo nor hi).");
            return T{};
        }
    }
}