#include "tz/lua/state.hpp"
extern "C"
{
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
}
#include <cstdint>

namespace tz::lua
{
	state::state(void* lstate):
	lstate(lstate){}

	bool state::valid() const
	{
		return this->lstate != nullptr;
	}

	bool state::execute_file(const char* path) const
	{
		return luaL_dofile(static_cast<lua_State*>(this->lstate), path) == false;
	}

	bool state::execute(const char* lua_src) const
	{
		return luaL_dostring(static_cast<lua_State*>(this->lstate), lua_src) == false;
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
