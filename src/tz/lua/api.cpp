#include "tz/lua/api.hpp"
#include "tz/dbgui/dbgui.hpp"
#include <sstream>
#include <filesystem>

// Module Lua API init
#include "tz/core/tz_core.hpp"
#include "tz/gl/tz_gl.hpp"
#include "tz/io/tz_io.hpp"
#include "tz/ren/tz_ren.hpp"
#include "tz/wsi/wsi.hpp"

extern "C"
{
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
}

namespace tz::lua
{
	LUA_BEGIN(assert)
		auto st = reinterpret_cast<lua_State*>(s);

		bool b = state.stack_get_bool(1);
		std::string stack = state.collect_stack();
		lua_Debug ar;
		lua_getstack(st, 1, &ar);
		lua_getinfo(st, "nSl", &ar);
		if(!b && TZ_DEBUG)
		{
			tz::dbgui::add_to_lua_log("<<Lua Assert Failure Detected>>");
		}
		tz::assert(b, "Lua Assertion Failure: ```lua\n\n%s\n\n```\nOn line %d\nStack:\n%s", ar.source, ar.currentline, stack.c_str());
		return 0;
	LUA_END

	LUA_BEGIN(error)
		auto st = reinterpret_cast<lua_State*>(s);
		lua_pushboolean(st, false);
		LUA_FN_NAME(assert)(st);
		state.stack_pop();
		return 0;
	LUA_END

	LUA_BEGIN(print)
		auto st = reinterpret_cast<lua_State*>(s);
		int nargs = state.stack_size();
		for(int i = 1; i <= nargs; i++)
		{
			std::string msg = state.stack_get_string(i, false);
			if(msg.empty())
			{
				lua_getglobal(st, "tostring");
				lua_pushvalue(st, i);
				lua_pcall(st, 1, 1, 0);
				msg = state.stack_get_string(-1);
				lua_pop(st, 1);
			}
			msg.erase(std::remove(msg.begin(), msg.end(), '\0'), msg.end());
			tz::dbgui::add_to_lua_log(msg);
		}
		tz::dbgui::add_to_lua_log("\n");
		state.stack_pop(nargs);
		return 0;
	LUA_END

	LUA_BEGIN(stack_dump)
		tz::dbgui::add_to_lua_log(state.collect_stack());
		return 0;
	LUA_END

	void api_initialise(state& s)
	{
		s.assign_emptytable("tz");
		s.assign_func("tz.assert", LUA_FN_NAME(assert));
		s.assign_func("tz.error", LUA_FN_NAME(error));
		LUA_REGISTER_ONE(print, s);
		LUA_REGISTER_ONE(stack_dump, s);
		s.assign_emptytable("tz.version");
		s.assign_emptytable("thread");

		std::ostringstream sstr;
		sstr << std::this_thread::get_id();
		s.assign_string("thread.id", sstr.str());
		s.assign_string("LUA_PATH", std::filesystem::current_path().generic_string());

		tz::version ver = tz::get_version();

		s.assign_uint("tz.version.major", ver.major);
		s.assign_uint("tz.version.minor", ver.minor);
		s.assign_uint("tz.version.patch", ver.patch);
		s.assign_string("tz.version.string", ver.to_string());

		tz::core::lua_initialise(s);
		tz::gl::lua_initialise(s);
		tz::io::lua_initialise(s);
		tz::ren::lua_initialise(s);
		tz::wsi::lua_initialise(s);
	}
}