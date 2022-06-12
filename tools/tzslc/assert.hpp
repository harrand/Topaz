#ifndef TZSLC_ASSERT_HPP
#define TZSLC_ASSERT_HPP
#include <cstdio>
#include <utility>

namespace tzslc
{
	namespace detail
	{
		struct AssertionError{const char* what;};
	}
	template<typename... Args>
	void error_internal([[maybe_unused]] const char* fmt, [[maybe_unused]] Args&&... args)
	{
			std::fflush(stderr);
			std::fprintf(stderr, fmt, std::forward<Args>(args)...);
			std::fflush(stderr);
			throw detail::AssertionError{"TZSLC Error"};
	}

	template<typename... Args>
	void assert_internal([[maybe_unused]] bool eval, [[maybe_unused]] const char* fmt, [[maybe_unused]] Args&&... args)
	{
			if(!eval)
			{
				std::fflush(stderr);
				std::fprintf(stderr, fmt, std::forward<Args>(args)...);
				std::fflush(stderr);
				throw detail::AssertionError{"TZSLC Assert"};
			}
	}
}

#ifdef tzslc_error
#undef tzslc_error
#endif

/**
 * @ingroup tzslc_core
 * @fn tzslc_error(msg, ...)
 * @hideinitializer
 * Causes a debug-only runtime error. Printf-like functionality which sends a formatted error message to stderr.
 */
#define tzslc_error(msg, ...) {tzslc::error_internal("tzslc_error:\nIn file %s:%d:\n\t " msg "\n", __FILE__, __LINE__, ##__VA_ARGS__);} (void)0;

#ifdef tzslc_assert
#undef tzslc_assert
#endif

/**
 * @ingroup tzslc_core
 * @fn tzslc_assert(eval, msg, ...)
 * @hideinitializer
 * Debug-only assert on some condition. If the condition evaluates to false, a runtime error is raised similar to @ref tzslc_error
 */
#define tzslc_assert(eval, msg, ...) {tzslc::assert_internal(eval, "tzslc_assert Failure: %s\nIn file: %s:%d:\n\t " msg "\n", #eval, __FILE__, __LINE__, ##__VA_ARGS__);} (void)0;

#endif // TZSLC_ASSERT_HPP
