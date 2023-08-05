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


	state& get_state()
	{
		if(!defstate.valid())
		{
			lua_State* l = luaL_newstate();
			luaL_openlibs(l);
			defstate = state{static_cast<void*>(l)};
		}
		return defstate;
	}
}
