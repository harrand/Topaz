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

#define tz_assert(cond, fmt, ...) if(!(cond)){tz::detail::error_internal("[Assertion Fail]: ", fmt, std::source_location::current(), __VA_ARGS__);}

#define tz_error(fmt, ...) tz::detail::error_internal("[Error]: ", fmt, std::source_location::current(), __VA_ARGS__)

#endif // TOPAZ_DEBUG_HPP