#ifndef TZ_DEBUG_HPP
#define TZ_DEBUG_HPP
#include <concepts>
#include <string>
#include <functional>

#include <version>
#ifdef __cpp_lib_source_location 
#include <source_location>
namespace tz::detail
{
	using source_loc = std::source_location;
}
#else
#pragma message("std::source_location support not detected, using a stub instead. Source information in report/assert/error will be wrong.")
#include <cstdint>
namespace tz::detail
{
	struct source_loc
	{
		static source_loc current(){return {};}
		constexpr std::uint_least32_t line() const{return 0;}
		constexpr std::uint_least32_t column() const{return 0;}
		constexpr const char* file_name() const{return "<Unknown>";}
		constexpr const char* function_name() const{return "<Unknown>";}
	};
}
#endif

#undef assert

namespace tz
{
	namespace detail
	{
		/// C-String wrapper with source-location information secretly present. From a user perspective, assume this is merely a `const char*`.
		struct format_string
		{
			const char* str;
			source_loc loc;

			constexpr format_string(const char* str, source_loc loc = source_loc::current()):
			str(str), loc(loc){}
		};
	}

	/**
 	 * @ingroup tz
	 * Assert on a condition. If the condition is false, `tz::error` is invoked. Only works on debug builds (aka `if TZ_DEBUG`).
	 */
	template<typename... Args>
	void assert(bool condition, detail::format_string fmt = "<No message>", Args&&... args);

	/**
	 * @ingroup tz
	 * Breakpoint if a debugger is present. If no debugger is present, terminate the application with the given message. Only works on debug builds (aka `if TZ_DEBUG`).
	 */ 
	template<typename... Args>
	void error(detail::format_string fmt = "<No message>", Args&&... args);

	/**
	 * @ingroup tz
	 * Print out the formatted message to standard output, including source location information. Only works on debug builds.
	 */ 
	template<typename... Args>
	void report(detail::format_string fmt, Args&&... args);

	void debug_break();
}

/**
 * Legacy assert. Provides similar functionality to `tz::assert`, although the calling notation directly matches `assert` from <cassert>.
 *
 * Some libraries allow you to define your own assert implementation. If so, it is recommended to define it to tz_legacy_assert if you want to be able to debug-break on assertions. Note that this is entirely optional, no tz behaviour requires this so exist.
 */
#define tz_legacy_assert(cond) tz::assert(cond, "Legacy Assert Detected")

#include "tz/core/debug.inl"
#endif // TZ_DEBUG_HPP
