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


#define tz_seterror(errcode, msg) set_last_error(std::format("({}): {}", error_code_name(errcode), msg))

/**
* @ingroup tz
* @brief Cause a runtime error if the expected value is erroneous. If not, the unwrapped expected value is returned.
*
* Many API functions in Topaz return some variant of `std::expected`. Handling them on an individual basis can be verbose and unnecessary. Surround the call with this macro to instead yield the expected value directly, and emit a runtime error if an error code was returned instead.
*
* @note You should only use this macro on a return value if you are happy to crash if the value is erroneous. Treat it as a glorified @ref tz_assert.
*
* Example before:

* `std::expected<tz::gpu::resource_handle, tz::error_code> img = tz::gpu::create_image({...});`

* Example after:

* `tz::gpu::resource_handle img = tz_must(tz::gpu::create_image({...}));`

* @hideinitializer
**/
#define tz_must(fnret) [sl = std::source_location::current()](auto ret){if(!ret.has_value()){tz::detail::error_internal("[Must Failure]: ", "error {} {}", sl, static_cast<int>(ret.error()), tz::last_error());} return ret.value();}(fnret)

#endif // TOPAZ_DEBUG_HPP