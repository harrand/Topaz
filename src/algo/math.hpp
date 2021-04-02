#ifndef TOPAZ_ALGO_MATH_HPP
#define TOPAZ_ALGO_MATH_HPP
#include "core/types.hpp"
#include <array>

namespace tz
{
	constexpr inline float pi = 3.14159265358979323846f;
	
	namespace algo
	{
		/**
		* \addtogroup tz_algo Topaz Algorithms Library (tz::algo)
		* Contains common algorithms used in Topaz modules that aren't available in the C++ standard library.
		* @{
		*/

		/**
		 * Perform a linear-interpolation between two known values.
		 * @tparam T Value type.
		 * @param a Known lower bound.
		 * @param b Known upper bound.
		 * @param weight Linear weight value.
		 * @return Interpolated unknwon value between `a` and `b`.
		 */
		constexpr auto linear_interpolate(tz::Number auto a, tz::Number auto b, tz::Number auto weight) -> decltype(weight);

		/**
		 * Perform a cubic-interpolation between two known values.
		 * @tparam T Value type.
		 * @param a Known lower bound.
		 * @param b Known upper bound.
		 * @param weight Cubic weight value.
		 * @return Interpolated unknwon value between `a` and `b`.
		 */
		constexpr auto cubic_interpolate(tz::Number auto a, tz::Number auto b, tz::Number auto weight) -> decltype(weight);

		/**
		 * Perform a cosine-interpolation between two known values.
		 * @tparam T Value type.
		 * @param a Known lower bound.
		 * @param b Known upper bound.
		 * @param weight Cosine weight value.
		 * @return Interpolated unknwon value between `a` and `b`.
		 */
		constexpr auto cosine_interpolate(tz::Number auto a, tz::Number auto b, tz::Number auto weight) -> decltype(weight);

		enum class SchmittBound
		{
			Lower,
			Higher
		};

		/**
		* Perform a schmitt invocation on a value. This can be seen as the logical complement to a clamp. Schmitt-triggers in electronics can be used to convert analogue signals to digital. This function acts similarly; saturates a value to a higher or lower result.
		* Note: There is one uniform type in this invocation. Mixing integers and floats for example will result in float truncation. To avoid this, use schmittf.
		* Example: schmitt(5, 10, 7, ...) yields 5. 5 is closer to 7 than 7 is to 10.
		* @tparam T Underlying value type to use.
		* @param lo Lowest value which should be returned.
		* @param hi Highest value which should be returned.
		* @param val Value to saturate.
		* @param bound Chosen behaviour if the value is equidistant between lo and hi.
		* @return If val is closer to lo, returns lo. If val was closer to hi, returns hi.
		*/
		constexpr auto schmitt(tz::Number auto lo, tz::Number auto hi, tz::Number auto val, SchmittBound bound = SchmittBound::Lower) -> decltype(hi - lo);

		/**
		* Perform a schmitt invocation on a value using the given factor.
		* Example: schmitt_multiple(8, 10, ...) yields 8. 10 is between the multiples 8 and 16. 10 is closer to 8, thus we return 8.
		* @tparam T Underlying value type to use.
		* @param factor Factor which the value will saturate to.
		* @param val Value to saturate.
		* @param bound Chosen behaviour if the value is equidistant between two multiples.
		* @return A multiple of 'factor' which 'val' is closest to.
		*/
		constexpr auto schmitt_multiple(tz::Number auto factor, tz::Number auto val, SchmittBound bound = SchmittBound::Lower);

		template<typename T>
		constexpr std::array<T, 3> axis_angle_to_euler(std::array<T, 3>, T angle);

		/**
		* @}
		*/
	}
}

#include "algo/math.inl"
#endif // TOPAZ_ALGO_MATH_HPP