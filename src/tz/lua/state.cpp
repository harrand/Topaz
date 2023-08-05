#include "tz/lua/state.hpp"
#include "tz/core/debug.hpp"
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
		 )lua") == false, "Failed to inject topaz lua state: %s", lua_tostring(state, -1));
		lua_getglobal(state, "tz");
		// tz.assert
		lua_pushcfunction(state, tz_lua_assert);
		lua_setfield(state, -2, "assert");
		lua_pop(state, 1);
	}
}
