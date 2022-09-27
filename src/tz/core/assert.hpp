#ifndef TOPAZ_CORE_ASSERT_HPP
#define TOPAZ_CORE_ASSERT_HPP
#include "tz/core/debugger.hpp"
#include <cstdio>
#include <cstdlib>
#include <utility>

namespace tz
{
	template<typename... Args>
	void error_internal([[maybe_unused]] const char* fmt, [[maybe_unused]] Args&&... args)
	{
		#if TZ_DEBUG
			std::fflush(stderr);
			std::fprintf(stderr, fmt, std::forward<Args>(args)...);
			std::fflush(stderr);
			tz::debug_break();
		#endif
	}

	template<typename... Args>
	void assert_internal([[maybe_unused]] bool eval, [[maybe_unused]] const char* fmt, [[maybe_unused]] Args&&... args)
	{
		#if TZ_DEBUG
			if(!eval)
			{
				std::fflush(stderr);
				std::fprintf(stderr, fmt, std::forward<Args>(args)...);
				std::fflush(stderr);
				tz::debug_break();
			}
		#endif
	}
}
#ifdef tz_error
#undef tz_error
#endif

/**
 * @ingroup tz_core
 * @fn tz_error(msg, ...)
 * @hideinitializer
 * Causes a debug-only runtime error. Printf-like functionality which sends a formatted error message to stderr.
 */
#define tz_error(msg, ...) {tz::error_internal("tz_error:\nIn file %s:%d:\n\t " msg "\n", __FILE__, __LINE__, ##__VA_ARGS__);} (void)0;

#ifdef tz_assert
#undef tz_assert
#endif

/**
 * @ingroup tz_core
 * @fn tz_assert(eval, msg, ...)
 * @hideinitializer
 * Debug-only assert on some condition. If the condition evaluates to false, a runtime error is raised similar to @ref tz_error
 */
#define tz_assert(eval, msg, ...) {tz::assert_internal(eval, "tz_assert Failure: %s\nIn file: %s:%d:\n\t " msg "\n", #eval, __FILE__, __LINE__, ##__VA_ARGS__);} (void)0;
#endif // TOPAZ_CORE_ASSERT_HPP
