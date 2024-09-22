#ifndef TOPAZ_DEBUG_HPP
#define TOPAZ_DEBUG_HPP
#include <source_location>
#include <cstdio>
#include <utility>
#include <format>
#include <string>

namespace tz::detail
{
	template<typename... Args>
	inline void error_internal(const char* preamble, std::format_string<Args...> fmt, const std::source_location& loc, Args&&... args)
	{
		std::fprintf(stderr, "%s", preamble);
		std::fprintf(stderr, "%s", std::format(fmt, std::forward<Args>(args)...).c_str());

		std::string diag_info = std::format("\n\tIn file {}({},{})\n\t>\t{}", loc.file_name(), loc.line(), loc.column(), loc.function_name());

		std::fprintf(stderr, "%s", diag_info.c_str());

		// debug break: int3
		asm("int3");
	}
}

/**
* @ingroup tz
* @brief Assert that the given condition must be true. Cause a #tz_error if not.
* @param cond Condition which must evaluate to true.
* @param fmt Format string, following the fmtlib convention (i.e the n'th instance of `{}` in the string will be replaced with the n'th variadic parameter).
* @param ... Additional arguments (size should correspond to the number of occurrences of `{}` in `fmt`) that shall be substituted into the format string.
* @hideinitializer
**/
#define tz_assert(cond, fmt, ...) if(!(cond)){tz::detail::error_internal("[Assertion Fail]: ", fmt, std::source_location::current(), __VA_ARGS__);}

/**
* @ingroup tz
* @brief Cause a runtime error. If a debugger is present, a breakpoint will occur at the call-site. Otherwise, the program will terminate.
* @param fmt Format string, following the fmtlib convention (i.e the n'th instance of `{}` in the string will be replaced with the n'th variadic parameter).
* @param ... Additional arguments (size should correspond to the number of occurrences of `{}` in `fmt`) that shall be substituted into the format string.
* @hideinitializer
**/
#define tz_error(fmt, ...) tz::detail::error_internal("[Error]: ", fmt, std::source_location::current(), __VA_ARGS__)

#endif // TOPAZ_DEBUG_HPP