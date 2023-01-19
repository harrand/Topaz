#ifndef TOPAZ_CORE_TIME_HPP
#define TOPAZ_CORE_TIME_HPP
#include "tz/core/types.hpp"
#include "tz/core/debug.hpp"
#include <compare>
#include <type_traits>

namespace tz
{
	/**
	 * @ingroup tz_core
	 * Represents some duration, expressable as a quantity of most time metrics.
	 */
	class duration
	{
	public:
		duration() = default;
		constexpr duration(unsigned long long nanos):
		duration_nanos(nanos){}

		/**
		 * Express the duration as some number of nanoseconds. Please be aware of possible precision issues if you're using floating point types.
		 * @return number of nanoseconds that this duration emcompasses.
		 */
		template<tz::number N>
		constexpr N nanos() const
		{
			return static_cast<N>(this->duration_nanos);
		}

		/**
		 * Express the duration as some number of microseconds. Please be aware of possible precision issues if you're using floating point types.
		 * @return number of microseconds that this duration emcompasses.
		 */
		template<tz::number N>
		constexpr N micros() const
		{
			return this->nanos<N>() / N{1000};
		}

		/**
		 * Express the duration as some number of milliseconds. Please be aware of possible precision issues if you're using floating point types.
		 * @return number of milliseconds that this duration emcompasses.
		 */
		template<tz::number N>
		constexpr N millis() const
		{
			return this->micros<N>() / N{1000};
		}

		/**
		 * Express the duration as some number of seconds. Please be aware of possible precision issues if you're using floating point types.
		 * @return number of seconds that this duration emcompasses.
		 */
		template<tz::number N>
		constexpr N seconds() const
		{
			return this->millis<N>() / N{1000};
		}

		/**
		 * Express the duration as some number of minutes. Please be aware of possible precision issues if you're using floating point types.
		 * @return number of minutes that this duration emcompasses.
		 */
		template<tz::number N>
		constexpr N minutes() const
		{
			return this->seconds<N>() / N{60};
		}

		/**
		 * Express the duration as some number of hours. Please be aware of possible precision issues if you're using floating point types.
		 * @return number of hours that this duration emcompasses.
		 */
		template<tz::number N>
		constexpr N hours() const
		{
			return this->minutes<N>() / N{60};
		}

		/**
		 * Express the duration as some number of days. Please be aware of possible precision issues if you're using floating point types.
		 * @return number of days that this duration emcompasses.
		 */
		template<tz::number N>
		constexpr N days() const
		{
			return this->hours<N>() / N{24};
		}

		constexpr auto operator<=>(const duration& rhs) const = default;
		constexpr duration& operator+=(const duration& rhs)
		{
			this->duration_nanos += rhs.duration_nanos;
			return *this;
		}

		constexpr duration& operator-=(const duration& rhs)
		{
			if(!std::is_constant_evaluated())
			{
				tz::assert(this->duration_nanos >= rhs.duration_nanos, "Cannot subtract a larger duration from a smaller duration, because durations cannot be negative. Please submit a bug report.");
			}
			this->duration_nanos -= rhs.duration_nanos;
			return *this;
		}

		constexpr duration operator+(const duration& rhs) const
		{
			return {this->duration_nanos + rhs.duration_nanos};
		}

		constexpr duration operator-(const duration& rhs) const
		{
			return {this->duration_nanos - rhs.duration_nanos};
		}
	private:
		unsigned long long duration_nanos = 0u;
	};

	namespace literals
	{
		constexpr duration operator""_ns(unsigned long long count)
		{
			return {count};
		}

		constexpr duration operator""_us(unsigned long long count)
		{
			return {count * 1000ull};
		}

		constexpr duration operator""_ms(unsigned long long count)
		{
			return {count * 1000000ull};
		}

		constexpr duration operator""_s(unsigned long long count)
		{
			return {count * 1000000000ull};
		}
	}

	/**
	 * @ingroup tz_core
	 * Retrieve a duration corresponding to the time passed since epoch.
	 */
	duration system_time();

	/**
	 * @ingroup tz_core
	 * An object which is falsy until a certain amount of time has passed since construction.
	 */
	class delay
	{
	public:
		/**
		 * Create a new delay object. The current system time is used.
		 * @param delay_length duration of the delay which should pass after construction before becoming truthy.
		 */
		delay(duration delay_length);
		/**
		 * Query as to whether the delay length has passed since construction of the delay object.
		 */
		bool done() const;
		/**
		 * Reset the delay object. The instant a delay object is reset, 0 seconds have elapsed since the "construction" of the object. You can use this to check for the same delay again in the future.
		 */
		void reset();

		operator bool() const{return this->done();}
	private:
		duration begin_systime;
		duration delay_length;
	};
}

#endif // TOPAZ_CORE_TIME_HPP
