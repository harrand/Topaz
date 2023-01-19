#include <utility>
#include <cstdio>
#include <cstdlib>
#include <string>

namespace tz
{
	template<typename... Args>
	void error_internal(const char* preamble, const char* fmt, const detail::source_loc& loc, Args&&... args)
	{
		std::fprintf(stderr, "%s", preamble);
		std::fprintf(stderr, fmt, std::forward<Args>(args)...);


		std::string diag_info = "\n\tIn file ";
		diag_info += std::string{loc.file_name()} + ":" + std::to_string(loc.line()) + ":" + std::to_string(loc.column()) + std::string{"\n\t>\t"} + loc.function_name();

		std::fprintf(stderr, "%s", diag_info.c_str());
		debug_break();
	}

	template<typename... Args>
	void report_internal(const char* preamble, const char* fmt, const detail::source_loc& loc, Args&&... args)
	{
		std::fprintf(stdout, "%s", preamble);
		std::fprintf(stdout, fmt, std::forward<Args>(args)...);
		std::string diag_info = "\n\tIn file ";
		diag_info += std::string{loc.file_name()} + ":" + std::to_string(loc.line()) + ":" + std::to_string(loc.column()) + std::string{"\n\t>\t"} + loc.function_name();
		std::fprintf(stdout, "%s\n", diag_info.c_str());
	}

	template<typename... Args>
	void assert([[maybe_unused]] bool condition, [[maybe_unused]] detail::format_string fmt, [[maybe_unused]] Args&&... args)
	{
		#if TZ_DEBUG
			if(!condition)
			{

				error_internal("[Assert Failure]: ", fmt.str, fmt.loc, std::forward<Args>(args)...);
			}
		#endif
	}

	template<typename... Args>
	void error([[maybe_unused]] detail::format_string fmt, [[maybe_unused]] Args&&... args)
	{
		#if TZ_DEBUG
			error_internal("[Error]: ", fmt.str, fmt.loc, std::forward<Args>(args)...);
		#endif
	}

	template<typename... Args>
	void report([[maybe_unused]] detail::format_string fmt, [[maybe_unused]] Args&&... args)
	{
		#if TZ_DEBUG
			report_internal("[Report]: ", fmt.str, fmt.loc, std::forward<Args>(args)...);
		#endif
	}

}
