#include "tz/lua/state.hpp"
#include "tz/lua/api.hpp"
#include "tz/core/debug.hpp"
#include "tz/core/data/version.hpp"
#include "tz/dbgui/dbgui.hpp"
#include "tz/core/job/job.hpp"
#include <iostream>
#include <sstream>
#include <cstdint>
#include <map>
#include <mutex>


extern "C"
{
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
}

// tracy has first class lua support \o/
#if TZ_PROFILE
#include "tracy/TracyLua.hpp"
#endif
#undef assert

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

	void state::assign_stack(const char* varname)
	{
		auto* s = static_cast<lua_State*>(this->lstate);
		lua_setglobal(s, varname);
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

	std::optional<std::string> state::get_string(const char* varname) const
	{
		std::optional<std::string> ret = std::nullopt;
		int stack_usage = 0;
		auto* s = static_cast<lua_State*>(this->lstate);
		if(impl_lua_get_var(varname, s, stack_usage) == LUA_TSTRING)
		{
			ret = std::string{lua_tostring(s, -1)};
		}
		lua_pop(s, stack_usage);
		return ret;
	}

	std::size_t state::stack_size() const
	{
		auto* s = static_cast<lua_State*>(this->lstate);
		return lua_gettop(s);
	}

	void state::stack_pop(std::size_t count)
	{
		auto* s = static_cast<lua_State*>(this->lstate);
		lua_pop(s, count);
	}

	bool state::stack_get_bool(std::size_t idx, bool type_check) const
	{
		auto* s = static_cast<lua_State*>(this->lstate);
		if(lua_isboolean(s, idx) || !type_check)
		{
			return lua_toboolean(s, idx);
		}
		else
		{
			std::string stackdata = this->collect_stack();
			std::string traceback = this->print_traceback();
			tz::error("Lua stack entry %zu requested as `bool`, type error. Stack:\n%s\nTraceback:\n%s", idx, stackdata.c_str(), traceback.c_str());
			return false;
		}
	}

	double state::stack_get_double(std::size_t idx, bool type_check) const
	{
		auto* s = static_cast<lua_State*>(this->lstate);
		if(lua_isnumber(s, idx) || !type_check)
		{
			return lua_tonumber(s, idx);
		}
		else
		{
			std::string stackdata = this->collect_stack();
			std::string traceback = this->print_traceback();
			tz::error("Lua stack entry %zu requested as `number`, type error. Stack:\n%s\nTraceback:\n%s", idx, stackdata.c_str(), traceback.c_str());
			return 0.0f;
		}
	}

	float state::stack_get_float(std::size_t idx, bool type_check) const
	{
		return static_cast<float>(this->stack_get_double(idx, type_check));
	}

	std::int64_t state::stack_get_int(std::size_t idx, bool type_check) const
	{
		return static_cast<std::int64_t>(this->stack_get_double(idx, type_check));
	}

	std::uint64_t state::stack_get_uint(std::size_t idx, bool type_check) const
	{
		return static_cast<std::uint64_t>(this->stack_get_double(idx, type_check));
	}

	std::string state::stack_get_string(std::size_t idx, bool type_check) const
	{
		auto* s = static_cast<lua_State*>(this->lstate);
		// important note: lua under-the-hood does implicit conversions to-and-from
		// string unless i explicitly disable them (e.g -DLUA_NOCVTN2S). this means
		// that type check will always succeed for strings, even when they are not.
		// however, i'm keeping the logic here still just for consistency.
		if(lua_isstring(s, idx) || !type_check)
		{
			std::size_t len;
			return {lua_tolstring(s, idx, &len), len};
		}
		else
		{
			std::string stackdata = this->collect_stack();
			std::string traceback = this->print_traceback();
			tz::error("Lua stack entry %zu requested as `string`, type error. Stack:\n%s\nTraceback:\n%s", idx, stackdata.c_str(), traceback.c_str());
			return "";
		}
	}

	void* state::stack_get_ptr(std::size_t idx, bool type_check) const
	{
		auto* s = static_cast<lua_State*>(this->lstate);
		if((lua_islightuserdata(s, idx) || lua_isuserdata(s, idx)) || !type_check)
		{
			return lua_touserdata(s, idx);
		}
		else
		{
			std::string stackdata = this->collect_stack();
			std::string traceback = this->print_traceback();
			tz::error("Lua stack entry %zu requested as `ptr`, type error. Stack:\n%s\nTraceback:\n%s", idx, stackdata.c_str(), traceback.c_str());
			return nullptr;
		}
	}

	lua_generic state::stack_get_generic(std::size_t idx) const
	{
		lua_generic ret;
		auto* s = static_cast<lua_State*>(this->lstate);
		if(lua_isboolean(s, idx))
		{
			ret = this->stack_get_bool(idx);
		}
		else if(lua_isinteger(s, idx))
		{
			ret = this->stack_get_int(idx);
		}
		else if(lua_isnumber(s, idx))
		{
			ret = this->stack_get_double(idx);
		}
		else if(lua_isstring(s, idx))
		{
			ret = this->stack_get_string(idx);
		}
		return ret;
	}

	void state::stack_push_nil() const
	{
		auto* s = static_cast<lua_State*>(this->lstate);
		lua_pushnil(s);
	}

	void state::stack_push_bool(bool b) const
	{
		auto* s = static_cast<lua_State*>(this->lstate);
		lua_pushboolean(s, b);
	}

	void state::stack_push_double(double d) const
	{
		auto* s = static_cast<lua_State*>(this->lstate);
		lua_pushnumber(s, d);
	}

	void state::stack_push_float(float f) const
	{
		this->stack_push_double(f);
	}

	void state::stack_push_int(std::int64_t i) const
	{
		this->stack_push_double(static_cast<double>(i));
	}

	void state::stack_push_uint(std::uint64_t u) const
	{
		this->stack_push_double(static_cast<double>(u));
	}

	void state::stack_push_string(std::string_view sv) const
	{
		auto* s = static_cast<lua_State*>(this->lstate);
		lua_pushlstring(s, sv.data(), sv.size());
	}

	void state::stack_push_generic(lua_generic generic) const
	{
		std::visit([this](auto&& arg)
		{
			using T = std::decay_t<decltype(arg)>;
			if constexpr(std::is_same_v<T, bool>)
			{
				this->stack_push_bool(arg);
			}
			else if constexpr(std::is_same_v<T, double>)
			{
				this->stack_push_double(arg);
			}
			else if constexpr(std::is_same_v<T, std::int64_t>)
			{
				this->stack_push_int(arg);
			}
			else if constexpr(std::is_same_v<T, std::string>)
			{
				this->stack_push_string(arg);
			}
			else if constexpr(std::is_same_v<T, tz::lua::nil>)
			{
				this->stack_push_nil();
			}
		}, generic);
	}

	void state::stack_push_ptr(void* ptr) const
	{
		auto* s = static_cast<lua_State*>(this->lstate);
		lua_pushlightuserdata(s, ptr);
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

	std::string state::print_traceback() const
	{
		this->execute("global_traceback_data = debug.traceback()");
		return this->get_string("global_traceback_data").value_or("no traceback data");
	}

	const std::string& state::get_last_error() const
	{
		return this->last_error;
	}

	std::thread::id state::get_owner_thread_id() const
	{
		return this->owner;
	}

	void state::attach_to_top_userdata(const char* classname, [[maybe_unused]] impl::lua_registers registers)
	{
		// whatever is on top of the stack, set it a new metatable with the functions.
		auto* s = static_cast<lua_State*>(this->lstate);
		luaL_getmetatable(s, classname);
		lua_setmetatable(s, -2);
	}

	void state::attach_to_top_table(impl::lua_registers registers)
	{
		auto* s = static_cast<lua_State*>(this->lstate);
		std::vector<luaL_Reg> regs;
		regs.resize(registers.size());
		for(std::size_t i = 0; i < registers.size(); i++)
		{
			regs[i] = luaL_Reg{.name = registers[i].namestr, .func = reinterpret_cast<lua_CFunction>(registers[i].fnptr)};
		}
		regs.push_back({.name = nullptr, .func = nullptr});
		luaL_setfuncs(s, regs.data(), 0);
	}

	void state::new_type(const char* type_name, impl::lua_registers registers)
	{
		tz::assert(registers.size());
		auto* s = static_cast<lua_State*>(this->lstate);
		luaL_checkversion(s);
		luaL_newmetatable(s, type_name);
		lua_pushstring(s, "__index");
		lua_pushvalue(s, -2);
		lua_settable(s, -3);
		this->attach_to_top_table(registers);
	}

	void state::open_lib(const char* name, impl::lua_registers registers)
	{
		tz::assert(registers.size());
		auto* s = static_cast<lua_State*>(this->lstate);
		luaL_checkversion(s);
		lua_newtable(s);
		this->attach_to_top_table(registers);
		lua_setglobal(s, name);
	}

	void* state::operator()() const
	{
		return this->lstate;
	}

	bool state::impl_check_stack(std::size_t sz) const
	{
		auto* s = static_cast<lua_State*>(this->lstate);
		return lua_checkstack(s, sz);
	}

	tz::memblk state::lua_userdata_stack_push(std::size_t byte_count) const
	{
		auto* s = static_cast<lua_State*>(this->lstate);
		return {.ptr = lua_newuserdata(s, byte_count), .size = byte_count};
	}

	thread_local state defstate = {};
	std::mutex state_creation_mtx;

	state& get_state()
	{
		if(!defstate.valid())
		{
			// luaL_newstate needs external synchronisation. once it's done though every thread has its own lua state.
			std::unique_lock<std::mutex> lock{state_creation_mtx};
			lua_State* l = luaL_newstate();
			luaL_openlibs(l);
			defstate = state{static_cast<void*>(l)};
			#if TZ_PROFILE
				tracy::LuaRegister(l);
			#else
				// registe no-op versions
				defstate.execute(R"(
					tracy =
					{
						ZoneBegin = function()
						end,
						ZoneEnd = function()
						end,
						ZoneBeginN = function()
						end,
						ZoneName = function()
						end,
						ZoneText = function()
						end,
					}
				)");
			#endif

			api_initialise(defstate);
		}
		return defstate;
	}

	void for_all_states(state_applicator fn)
	{
		// for each worker, execute a new job to register the function with the necessary affinity.
		std::vector<tz::job_handle> handles = {};
		for(tz::worker_id_t wid : tz::job_system().get_worker_ids())
		{
			handles.push_back(tz::job_system().execute([fn]()
			{
				fn(get_state());
			}, {.maybe_worker_affinity = wid}));
		}
		// dont return till its all done.
		for(tz::job_handle jh : handles)
		{
			tz::job_system().block(jh);
		}
		// remember also add it for main thread (which is us).
		fn(get_state());
	}
}
