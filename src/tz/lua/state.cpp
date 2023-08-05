#include "tz/lua/state.hpp"
#include "tz/core/debug.hpp"
#include "tz/core/data/version.hpp"
#include <cstdint>

extern "C"
{
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
}

namespace tz::lua
{
	state::state(void* lstate):
	lstate(lstate){}

	bool state::valid() const
	{
		return this->lstate != nullptr;
	}

	bool state::execute_file(const char* path, bool assert_on_failure) const
	{
		auto* s = static_cast<lua_State*>(this->lstate);
		bool ret = luaL_dofile(s, path) == false;
		tz::assert(!assert_on_failure || ret, "Lua Error: %s", lua_tostring(s, -1));
		return ret;
	}

	bool state::execute(const char* lua_src, bool assert_on_failure) const
	{
		auto* s = static_cast<lua_State*>(this->lstate);
		bool ret = luaL_dostring(s, lua_src) == false;
		tz::assert(!assert_on_failure || ret, "Lua Error: %s", lua_tostring(s, -1));
		return ret;
	}

	bool state::define_nil(const char* varname) const
	{
		bool ret = impl_check_stack(1);
		auto* s = static_cast<lua_State*>(this->lstate);
		lua_pushnil(s);
		lua_setglobal(s, varname);
		return ret;
	}

	bool state::define_bool(const char* varname, bool b) const
	{
		bool ret = impl_check_stack(1);
		auto* s = static_cast<lua_State*>(this->lstate);
		lua_pushboolean(s, b);
		lua_setglobal(s, varname);
		return ret;
	}

	bool state::define_float(const char* varname, float f) const
	{
		return this->define_double(varname, f);
	}

	bool state::define_double(const char* varname, double d) const
	{
		bool ret = impl_check_stack(1);
		auto* s = static_cast<lua_State*>(this->lstate);
		lua_pushnumber(s, d);
		lua_setglobal(s, varname);
		return ret;
	}

	bool state::define_int(const char* varname, std::int64_t i) const
	{
		bool ret = impl_check_stack(1);
		auto* s = static_cast<lua_State*>(this->lstate);
		lua_pushinteger(s, i);
		lua_setglobal(s, varname);
		return ret;
	}

	bool state::define_uint(const char* varname, std::uint64_t u) const
	{
		// ugh...
		return this->define_int(varname, static_cast<std::int64_t>(u));
	}

	bool state::define_func(const char* varname, void* func_ptr) const
	{
		bool ret = impl_check_stack(1);
		auto* s = static_cast<lua_State*>(this->lstate);
		lua_pushcfunction(s, reinterpret_cast<lua_CFunction>(func_ptr));
		lua_setglobal(s, varname);
		return ret;
	}

	bool state::impl_check_stack(std::size_t sz) const
	{
		auto* s = static_cast<lua_State*>(this->lstate);
		return lua_checkstack(s, sz);
	}

	state defstate = {};

	void tz_inject_state(lua_State* state);

	state& get_state()
	{
		if(!defstate.valid())
		{
			lua_State* l = luaL_newstate();
			luaL_openlibs(l);
			tz_inject_state(l);
			defstate = state{static_cast<void*>(l)};
		}
		return defstate;
	}

	int tz_lua_assert(lua_State* state)
	{
		bool b = lua_toboolean(state, 1);
		lua_Debug ar;
		lua_getstack(state, 1, &ar);
		lua_getinfo(state, "nSl", &ar);
		tz::assert(b, "Lua Assertion: ```lua\n\n%s\n\n```\nOn line %d", ar.source, ar.currentline);
		return 0;
	}

	void tz_inject_state(lua_State* state)
	{
		tz::assert(luaL_dostring(state, R"lua(
			tz = {}
			tz.version = {}
		 )lua") == false, "Failed to inject topaz lua state: %s", lua_tostring(state, -1));
		lua_getglobal(state, "tz");
		// tz.assert
		lua_pushcfunction(state, tz_lua_assert);
		lua_setfield(state, -2, "assert");

		tz::version ver = tz::get_version();

		// tz.version = {.major = X, .minor = Y, .patch = Z, .string = "vX.Y.Z-something"}
		lua_newtable(state);
		lua_pushstring(state, "major");
		lua_pushinteger(state, ver.major);
		lua_settable(state, -3);

		lua_pushstring(state, "minor");
		lua_pushinteger(state, ver.minor);
		lua_settable(state, -3);
		
		lua_pushstring(state, "patch");
		lua_pushinteger(state, ver.patch);
		lua_settable(state, -3);

		lua_pushstring(state, "string");
		lua_pushstring(state, ver.to_string().c_str());
		lua_settable(state, -3);

		lua_setfield(state, -2, "version");
		lua_pop(state, 1);
	}
}
