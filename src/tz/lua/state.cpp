#include "tz/lua/state.hpp"
#include "tz/core/debug.hpp"
#include "tz/core/data/version.hpp"
#include "tz/dbgui/dbgui.hpp"
#include <sstream>
#include <iostream>
#include <cstdint>
#include <map>
#include <filesystem>

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
		const char* err = lua_tostring(s, -1);
		if(err != nullptr)
		{
			this->last_error = err;
		}
		tz::assert(!assert_on_failure || ret, "Lua Error: %s", err);
		return ret;
	}

	bool state::execute(const char* lua_src, bool assert_on_failure) const
	{
		auto* s = static_cast<lua_State*>(this->lstate);
		bool ret = luaL_dostring(s, lua_src) == false;
		const char* err = lua_tostring(s, -1);
		if(err != nullptr)
		{
			this->last_error = err;
		}
		tz::assert(!assert_on_failure || ret, "Lua Error: %s", err);
		return ret;
	}

	bool state::assign_nil(const char* varname) const
	{
		std::string cmd = std::string(varname) + " = nil";
		return this->execute(cmd.c_str(), false);
	}

	bool state::assign_emptytable(const char* varname) const
	{
		std::string cmd = std::string(varname) + " = {}";
		return this->execute(cmd.c_str(), false);
	}

	bool state::assign_bool(const char* varname, bool b) const
	{
		std::string cmd = std::string(varname) + " = " + (b ? "true" : "false");
		return this->execute(cmd.c_str(), false);
	}

	bool state::assign_float(const char* varname, float f) const
	{
		return this->assign_double(varname, f);
	}

	bool state::assign_double(const char* varname, double d) const
	{
		std::string cmd = std::string(varname) + " = " + std::to_string(d);
		return this->execute(cmd.c_str(), false);
	}

	bool state::assign_int(const char* varname, std::int64_t i) const
	{
		std::string cmd = std::string(varname) + " = " + std::to_string(i);
		return this->execute(cmd.c_str(), false);
	}

	bool state::assign_uint(const char* varname, std::uint64_t u) const
	{
		// ugh...
		return this->assign_int(varname, static_cast<std::int64_t>(u));
	}

	bool state::assign_func(const char* varname, void* func_ptr) const
	{
		bool ret = impl_check_stack(1);
		auto* s = static_cast<lua_State*>(this->lstate);
		std::string temp_varname = "tmp" + std::to_string(reinterpret_cast<std::intptr_t>(func_ptr));
		lua_pushcfunction(s, reinterpret_cast<lua_CFunction>(func_ptr));
		lua_setglobal(s, temp_varname.c_str());
		std::string cmd = std::string(varname) + " = " + temp_varname;
		ret &= this->execute(cmd.c_str(), false);
		return ret;
	}

	bool state::assign_string(const char* varname, std::string str) const
	{
		std::string cmd = std::string(varname) + " = \"" + str + "\"";
		return this->execute(cmd.c_str(), false);
	}

	std::vector<std::string> impl_string_split(const std::string& str, const std::string& delim)
	{
		std::vector<std::string> result;
		std::size_t start = 0;

		for(std::size_t found = str.find(delim); found != std::string::npos; found = str.find(delim, start))
		{
			result.emplace_back(str.begin() + start, str.begin() + found);
			start = found + delim.size();
		}
		if (start != str.size())
			result.emplace_back(str.begin() + start, str.end());
		return result;      
	}

	// leaves 1 extra on the stack.
	int impl_lua_get_var(std::string varname, lua_State* s, int& stack_sz)
	{
		auto bits = impl_string_split(varname, ".");		
		tz::assert(bits.size());
		int type = lua_getglobal(s, bits.front().c_str());
		bits.erase(bits.begin());
		stack_sz++;
		for(const std::string& bit : bits)
		{
			type = lua_getfield(s, -1, bit.c_str());
			stack_sz++;
			if(type == LUA_TNIL)
			{
				break;
			}
		}
		return type;	
	}

	std::optional<bool> state::get_bool(const char* varname) const
	{
		std::optional<bool> ret = std::nullopt;
		int stack_usage = 0;
		auto* s = static_cast<lua_State*>(this->lstate);
		if(impl_lua_get_var(varname, s, stack_usage) == LUA_TBOOLEAN)
		{
			ret = lua_toboolean(s, -1);
		}
		lua_pop(s, stack_usage);
		return ret;
	}

	std::optional<float> state::get_float(const char* varname) const
	{
		auto ret = this->get_double(varname);
		if(ret.has_value())
		{
			return static_cast<float>(ret.value());
		}
		return std::nullopt;
	}

	std::optional<double> state::get_double(const char* varname) const
	{
		std::optional<double> ret = std::nullopt;
		int stack_usage = 0;
		auto* s = static_cast<lua_State*>(this->lstate);
		if(impl_lua_get_var(varname, s, stack_usage) == LUA_TNUMBER)
		{
			ret = lua_tonumber(s, -1);
		}
		lua_pop(s, stack_usage);
		return ret;
	}

	std::optional<std::int64_t> state::get_int(const char* varname) const
	{
		auto ret = this->get_double(varname);
		if(ret.has_value())
		{
			return static_cast<std::int64_t>(ret.value());
		}
		return std::nullopt;
	}

	std::optional<std::uint64_t> state::get_uint(const char* varname) const
	{
		auto ret = this->get_int(varname);
		if(ret.has_value())
		{
			return static_cast<std::uint64_t>(ret.value());
		}
		return std::nullopt;
	}

	std::string state::collect_stack() const
	{
		if(!this->impl_check_stack(3))
		{
			return "<cant collect - oom>";
		}
		std::string ret;

		auto* s = static_cast<lua_State*>(this->lstate);
		int top = lua_gettop(s);
		int bottom = 1;
		ret = "=== stack (size: " + std::to_string(top - bottom + 1) + ") ===\n";
		lua_getglobal(s, "tostring");
		for(int i = top; i >= bottom; i--)
		{
			lua_pushvalue(s, -1);
			lua_pushvalue(s, i);
			lua_pcall(s, 1, 1, 0);
			const char* str = lua_tostring(s, -1);
			ret += std::string(">") + std::to_string(i) + ": ";
			if(str == nullptr)
			{
				ret += luaL_typename(s, i) + std::string("\n");
			}
			else
			{
				ret += str + std::string("\n");
			}
			lua_pop(s, 1);
		}
		lua_pop(s, 1);
		return ret + "=== end ===";
	}

	const std::string& state::get_last_error() const
	{
		return this->last_error;
	}

	bool state::impl_check_stack(std::size_t sz) const
	{
		auto* s = static_cast<lua_State*>(this->lstate);
		return lua_checkstack(s, sz);
	}

	state defstate = {};

	void tz_inject_state(state& s);

	state& get_state()
	{
		if(!defstate.valid())
		{
			lua_State* l = luaL_newstate();
			luaL_openlibs(l);
			defstate = state{static_cast<void*>(l)};

			tz_inject_state(defstate);
		}
		return defstate;
	}

	int tz_lua_assert(lua_State* state)
	{
		bool b = lua_toboolean(state, 1);
		std::string stack = lua::state{state}.collect_stack();
		lua_Debug ar;
		lua_getstack(state, 1, &ar);
		lua_getinfo(state, "nSl", &ar);
		if(!b && TZ_DEBUG)
		{
			tz::dbgui::add_to_lua_log("<<Lua Assert Failure Detected>>");
		}
		tz::assert(b, "Lua Assertion Failure: ```lua\n\n%s\n\n```\nOn line %d\nStack:\n%s", ar.source, ar.currentline, stack.c_str());
		return 0;
	}

	int tz_print(lua_State* state)
	{
		int nargs = lua_gettop(state);
		for(int i = 1; i <= nargs; i++)
		{
			const char* msg = luaL_tolstring(state, i, nullptr);
			if(msg != nullptr)
			{
				tz::dbgui::add_to_lua_log(msg);
			}
		}
		tz::dbgui::add_to_lua_log("\n");
		lua_pop(state, nargs);
		return 0;
	}

	void tz_inject_state(state& s)
	{
		s.assign_emptytable("tz");
		s.assign_emptytable("tz.version");
		s.assign_func("tz.assert", tz_lua_assert);	
		s.assign_func("print", tz_print);

		s.assign_string("LUA_PATH", std::filesystem::current_path().generic_string());

		tz::version ver = tz::get_version();

		s.assign_uint("tz.version.major", ver.major);
		s.assign_uint("tz.version.minor", ver.minor);
		s.assign_uint("tz.version.patch", ver.patch);
		s.assign_string("tz.version.string", ver.to_string());
	}
}
