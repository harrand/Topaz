#ifndef TOPAZ_ALGO_MATH_HPP
#define TOPAZ_ALGO_MATH_HPP

namespace tz::algo
{
    /**
     * \addtogroup tz_algo Topaz Algorithms Library (tz::algo)
     * Contains common algorithms used in Topaz modules that aren't available in the C++ standard library.
     * @{
     */

    enum class SchmittBound
    {
        Lower,
        Higher
    };

    /**
     * Perform a schmitt invocation on a value. Schmitt-triggers in electronics can be used to convert analogue signals to digital. This function acts similarly; saturates a value to a higher or lower result.
     * Note: There is one uniform type in this invocation. Mixing integers and floats for example will result in float truncation. To avoid this, use schmittf.
     * @tparam T Underlying value type to use.
     * @param lo Lowest value which should be returned.
     * @param hi Highest value which should be returned.
     * @param val Value to saturate.
     * @param bound Chosen behaviour if the value is equisdistant between lo and hi.
     * @return If val is closer to lo, returns lo. If val was closer to hi, returns hi.
     */
    template<typename T>
    T schmitt(T lo, T hi, T val, SchmittBound bound = SchmittBound::Lower);

    /**
     * Perform a schmitt invocation on a value. Schmitt-triggers in electronics can be used to convert analogue signals to digital. This function acts similarly; saturates a value to a higher or lower result.
     * Note: This has more overhead than schmitt. If a single uniform type will suffice (no fear of truncation for example), then use schmitt.
     * @tparam T Underlying value type to use. This might want to be int.
     * @tparam F Underlying second value type to use. This might want to be float.
     * @param lo Lowest value which should be returned.
     * @param hi Highest value which should be returned.
     * @param val Value to saturate.
     * @param bound Chosen behaviour if the value is equisdistant between lo and hi.
     * @return If val is closer to lo, returns lo. If val was closer to hi, returns hi.
     */
    template<typename T, typename F>
    T schmittf(T lo, T hi, F val, SchmittBound bound = SchmittBound::Lower);

    /**
     * @}
     */
}

#include "algo/math.inl"
#endif // TOPAZ_ALGO_MATH_HPP