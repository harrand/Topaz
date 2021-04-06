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
		 * @name Mathematical Algorithms
		 */
		///@{

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
		* Perform a binary clamp on a value between `lo` and `hi`.
		* A typical clamp will take any value and return a value between `lo` and `hi`. A binary clamp will instead take any value between `lo` and `hi` and return a value equal to either `lo` or `hi`, depending on which one is closer to the value.
		* Example: binary_clamp(5, 10, 7, Lower) yields 5. 5 is closer to 7 than 7 is to 10. If the Higher bound is instead used, the returned value will be 10.
		* @tparam T Underlying value type to use.
		* @param lo Lowest value which could be returned.
		* @param hi Highest value which could be returned.
		* @param val Value to saturate.
		* @param bound Chosen behaviour if the value is equidistant between lo and hi.
		* @return If val is closer to lo, returns lo. If val was closer to hi, returns hi.
		*/
		constexpr auto binary_clamp(tz::Number auto lo, tz::Number auto hi, tz::Number auto val, SchmittBound bound = SchmittBound::Lower) -> decltype(hi - lo);

		/**
		* This clamp will take a value and a factor, and return a multiple of `factor` that was closest to `val`.
		* Example: clamp_by_factor(8, 10, Lower) yields 8. 10 is between the multiples 8 and 16, but is closer to 8. If the Higher bound is instead used, 16 is returned.
		* @tparam T Underlying value type to use.
		* @param factor Factor which the value will saturate to.
		* @param val Value to saturate.
		* @param bound Chosen behaviour if the value is equidistant between two multiples.
		* @return A multiple of 'factor' which 'val' is closest to.
		*/
		constexpr auto clamp_by_factor(tz::Number auto factor, tz::Number auto val, SchmittBound bound = SchmittBound::Lower);

		/**
		 * Interpret an array of numbers as an axis-angle, and return a similar array representing the equivalent set of euler-angles for that rotation.
		 */
		template<tz::Number T>
		constexpr std::array<T, 3> axis_angle_to_euler(std::array<T, 3>, T angle);

		///@}

		/**
		* @}
		*/
	}
}

#include "algo/math.inl"
#endif // TOPAZ_ALGO_MATH_HPP