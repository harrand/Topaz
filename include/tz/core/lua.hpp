#ifndef TOPAZ_LUA_HPP
#define TOPAZ_LUA_HPP
#include "tz/core/error.hpp"
#include <filesystem>

namespace tz
{
	using lua_fn = int(*)(void*);
	/**
	 * @ingroup tz_core
	 * Attempt to execute a local lua file on the current thread.
	 * @param path Path to a local file containing lua code.
	 *
	 * @return @ref tz::error_code::precondition_failure If the provided path was invalid.
	 * @return @ref tz::error_code::unknown_error If the executed code caused an error.
	 */
	tz::error_code lua_execute_file(std::filesystem::path path);
	/**
	 * @ingroup tz_core
	 * Attempt to execute some lua code on the current thread.
	 * @param lua_src String containing lua code to execute.
	 *
	 * @return @ref tz::error_code::unknown_error If the executed code caused an error.
	 */
	tz::error_code lua_execute(std::string_view lua_src);

	tz::error_code lua_set_nil(std::string_view varname);
	tz::error_code lua_set_emptytable(std::string_view varname);
	tz::error_code lua_set_bool(std::string_view varname, bool v);
	tz::error_code lua_set_int(std::string_view varname, std::int64_t v);
	tz::error_code lua_set_number(std::string_view varname, double v);
	tz::error_code lua_define_function(std::string_view varname, lua_fn fn);
}

#endif // TOPAZ_LUA_HPP