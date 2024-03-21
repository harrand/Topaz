module;
#include <version>
#include <cstdint>
#include <source_location>
#include <iostream>
#include <format>
export module topaz.debug;

namespace tz::debug
{
	struct fake_source_location
	{
		static fake_source_location current(){return {};}
		constexpr std::uint_least32_t line() const{return 0;}
		constexpr std::uint_least32_t column() const{return 0;}
		constexpr const char* file_name() const{return "<Unknown>";}
		constexpr const char* function_name() const{return "<Unknown>";}
	};

	#ifdef __cpp_lib_source_location 
		using source_location = std::source_location;
	#else
		using source_location = fake_source_location;
	#endif
	
	struct format_cstring
	{
		const char* cstr;
		source_location loc = source_location::current();

		constexpr format_cstring(const char* str, source_location loc = source_location::current()):
			cstr(str), loc(loc){}
	};

	export void error(format_cstring fmt = "<No Message>")
	{
		std::cerr << fmt.cstr << "\n";

		std::cerr << "\n\tIn file " << std::format("{}:{}:{}\n\t>{}",
			fmt.loc.file_name(), fmt.loc.line(), fmt.loc.column(), fmt.loc.function_name());

		// TODO: C++26 => use std::breakpoint.
		// Possible alternative: scottt/debugbreak, but I want minimal code and minimal dependencies. So int3 it is.
		asm("int3");
	}

	export void assert_that(bool expression, format_cstring fmt = "<No Message>")
	{
		#if TOPAZ_DEBUG
		if(!expression)
		{
			error(fmt);
		}
		#endif
	}
}
