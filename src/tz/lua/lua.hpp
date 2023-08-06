#ifndef TOPAZ_LUA_LUA_HPP
#define TOPAZ_LUA_LUA_HPP

/**
* @page tz_lua Lua API Reference
* @section Introduction
* - During the runtime of a debug-build, you can use the lua console within the dbgui to run lua ad-hoc (location: "Engine->Lua Console")
* @section Functions
* @code{.lua}
* function tz.assert(expr: boolean)
* @endcode
* Asserts that an expression is true. If it is false, an assertion failure occurs on debug builds.
* <hr>
*
* @section Constants
* @code{.lua}
* table tz.version
* {
* 	number major,
* 	number minor,
* 	number patch,
* 	string string
* }
* @endcode
* A table containing version info for the current version of the host engine.
* @note `tz.version.string` represents a readable, printable Topaz version.
*
* <hr>
*/

/**
* @ingroup tz_cpp
* @defgroup tz_lua_cpp Lua Integration
* Run Lua code from within Topaz C++. See @ref tz_lua for the Topaz-provided lua API.
*/

#endif // TOPAZ_LUA_LUA_HPP
