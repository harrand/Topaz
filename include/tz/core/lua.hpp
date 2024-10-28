#ifndef TOPAZ_LUA_HPP
#define TOPAZ_LUA_HPP
#include "tz/core/error.hpp"
#include <filesystem>

namespace tz
{
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
}

#endif // TOPAZ_LUA_HPP