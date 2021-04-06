#include <cmath>

namespace tz::algo
{
	constexpr auto linear_interpolate(tz::Number auto a, tz::Number auto b, tz::Number auto weight) -> decltype(weight)
	{
		return (b - a) * weight + a;
	}

	constexpr auto cubic_interpolate(tz::Number auto a, tz::Number auto b, tz::Number auto weight) -> decltype(weight)
	{
		return (b - a) * (3 - weight * 2) * weight * weight + a;
	}

	constexpr auto cosine_interpolate(tz::Number auto a, tz::Number auto b, tz::Number auto weight) -> decltype(weight)
	{
		auto theta = weight * tz::pi;
		// get a value between 0.0-1.0. then pass into linear interpolate.
		auto clamped_value = 1 - std::cos(theta) * 0.5f;
		return linear_interpolate(a, b, clamped_value);
	}

	constexpr auto binary_clamp(tz::Number auto lo, tz::Number auto hi, tz::Number auto val, SchmittBound bound) -> decltype(hi - lo)
	{
		tz::Number auto hdist = hi - val;
		tz::Number auto ldist = val - lo;
		if constexpr(std::is_signed_v<decltype(hdist)>)
		{
			hdist = std::abs(hdist);
		}
		if constexpr(std::is_signed_v<decltype(ldist)>)
		{
			ldist = std::abs(ldist);
		}

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
			topaz_assert(false, "tz::algo::binary_clamp(", lo, ", ", hi, ", ...): SchmittBound unrecognised (not lo nor hi).");
			return {};
		}
	}

	constexpr auto clamp_by_factor(tz::Number auto factor, tz::Number auto value, SchmittBound bound)
	{
		topaz_assert(factor != 0, "tz::algo::clamp_by_factor(", factor, ", ", value, ", ...): Cannot divide by factor ", factor);
		tz::Number auto frac = std::floor(value / factor);
		tz::Number auto lo = frac * factor;
		tz::Number auto hi = (frac + 1) * factor;
		return tz::algo::binary_clamp(lo, hi, value, bound);
	}

	template<tz::Number T>
	constexpr std::array<T, 3> axis_angle_to_euler(std::array<T, 3> axis, T angle)
	{
		T s = std::sin(angle);
		T c = std::cos(angle);
		T t = T{1} - c;
		// Assume axis is normalised already.
		T x = axis[0];
		T y = axis[1];
		T z = axis[2];
		T singularity = ((x * y * t) + (z * s));
		T ex, ey, ez;
		if(singularity > 0.998)
		{
			// north pole singularity
			ey = T{2} * std::atan2(x * std::sin(angle / T{2}), std::cos(angle / T{2}));
			ex = std::asin(1); // pi/2
			ez = T{};
			return {ex, ey, ez};
		}
		if(singularity < -0.998)
		{
			ey = T{-2} * std::atan2(x * std::sin(angle / T{2}), std::cos(angle / T{2}));
			ex = -std::asin(1); // -pi/2
			ez = T{};
			return {ex, ey, ez};
		}
		ey = std::atan2(y * s - x * z * t, T{1} - (y * y + z * z) * t);
		ex = std::asin(x * y * t + z * s);
		ez = -std::atan2(x * s - y * z * t, T{1} - (x * x + z * z) * t);
		return {ex, ey, ez};
	}

}