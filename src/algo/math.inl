#include <cmath>

namespace tz::algo
{
	template<typename T>
	T linear_interpolate(T a, T b, T weight)
	{
		return (b - a) * weight + a;
	}

	template<typename T>
	T cubic_interpolate(T a, T b, T weight)
	{
		return (b - a) * (T{3} - weight * T{2}) * weight * weight + a;
	}

	template<typename T>
	T cosine_interpolate(T a, T b, T weight)
	{
		T theta = weight * tz::pi;
		// get a value between 0.0-1.0. then pass into linear interpolate.
		T clamped_value = T{1} - std::cos(theta) * T{0.5f};
		return linear_interpolate<T>(a, b, clamped_value);
	}

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

	template<typename T>
	T schmitt_multiple(T factor, T value, SchmittBound bound)
	{
		topaz_assert(factor != 0, "tz::algo::schmitt_multiple(", factor, ", ", value, ", ...): Cannot divide by factor ", factor);
		T frac = std::floor(value / factor);
		T lo = frac * factor;
		T hi = (frac + 1) * factor;
		return tz::algo::schmitt<T>(lo, hi, value, bound);
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

	template<typename T, typename F>
	T schmittf_multiple(T factor, F value, SchmittBound bound)
	{
		topaz_assert(factor != 0, "tz::algo::schmitt_multiple(", factor, ", ", value, ", ...): Cannot divide by factor ", factor);
		F frac = std::floor(value / static_cast<F>(factor));
		T lo = frac * factor;
		T hi = (frac + 1) * factor;
		return tz::algo::schmittf<T, F>(lo, hi, value, bound);
	}

	template<typename T>
	std::array<T, 3> axis_angle_to_euler(std::array<T, 3> axis, T angle)
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