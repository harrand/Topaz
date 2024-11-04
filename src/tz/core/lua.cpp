#include "tz/core/lua.hpp"
#include "tz/topaz.hpp"

#include "tz/core/job.hpp"
#include <any>

extern "C"
{
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
}

namespace tz
{
	thread_local lua_State* lua;

	namespace detail
	{
		void lua_initialise_local()
		{
			lua = luaL_newstate();
			luaL_openlibs(lua);
		}

		void lua_initialise_all_threads()
		{
			std::vector<tz::job_handle> jobs;
			jobs.resize(tz::job_worker_count());
			for(std::size_t i = 0; i < jobs.size(); i++)
			{
				jobs[i] = tz::job_execute_on(lua_initialise_local, i);
			}
			lua_initialise_local();
			for(tz::job_handle job : jobs)
			{
				tz::job_wait(job);
			}
		}
	}

	tz::error_code lua_execute_file(std::filesystem::path path)
	{
		if(!std::filesystem::exists(path))
		{
			RETERR(tz::error_code::precondition_failure, "path to supposed lua file {} is invalid", path.string());
		}
		luaL_dofile(lua, path.string().c_str());
		const char* err = lua_tostring(lua, -1);
		if(err != nullptr)
		{
			RETERR(tz::error_code::unknown_error, "lua error while executing file {}: {}", path.filename().string(), err);
		}
		return tz::error_code::success;
	}

	tz::error_code lua_execute(std::string_view lua_src)
	{
		bool ret = luaL_dostring(lua, lua_src.data()) == false;
		const char* err = lua_tostring(lua, -1);
		if(!ret)
		{
			std::string code_snippet{lua_src.data(), lua_src.data() + std::min(20uz, lua_src.size())};
			RETERR(tz::error_code::unknown_error, "lua error while executing code \"{}...\": {}\n\tdetails:\n{}\n{}", code_snippet.c_str(), err != nullptr ? err : "<no error message>", lua_debug_stack(), lua_debug_callstack());
		}
		return tz::error_code::success;
	}

	tz::error_code lua_set_nil(std::string_view varname)
	{
		return lua_execute(std::format("{} = nil", varname));
	}

	tz::error_code lua_set_emptytable(std::string_view varname)
	{
		return lua_execute(std::format("{} = {{}}", varname));
	}

	tz::error_code lua_set_bool(std::string_view varname, bool v)
	{
		return lua_execute(std::format("{} = {}", varname, v ? "true" : "false"));
	}

	tz::error_code lua_set_int(std::string_view varname, std::int64_t v)
	{
		return lua_execute(std::format("{} = {}", varname, v));
	}

	tz::error_code lua_set_number(std::string_view varname, double v)
	{
		return lua_execute(std::format("{} = {}", varname, v));
	}

	tz::error_code lua_set_string(std::string_view varname, std::string v)
	{
		return lua_execute(std::format("{} = \"{}\"", varname, v));
	}

	tz::error_code lua_define_function(std::string_view varname, lua_fn fn)
	{
		lua_pushcfunction(lua, reinterpret_cast<lua_CFunction>(fn));
		std::string tmp = std::format("tmp_{}", varname);
		lua_setglobal(lua, tmp.c_str());
		lua_execute(std::format("{} = {}", varname, tmp));
		return tz::error_code::success;
	}

	int impl_lua_get_var(std::string_view varname, int& stack_sz);

	#define GET_IMPL(api_typename, cpp_typename, lua_typeid, lua_convfn) std::expected<cpp_typename, tz::error_code> lua_get_##api_typename(std::string_view varname){\
		int stack_usage = 0;\
		auto innerfn = [&varname, &stack_usage]()->std::expected<cpp_typename, tz::error_code>{\
			if(impl_lua_get_var(varname, stack_usage) == lua_typeid)\
			{\
				return static_cast<cpp_typename>(lua_convfn(lua, -1));\
			}\
			else\
			{\
				UNERR(tz::error_code::precondition_failure, "variable \"{}\" was requested as type \"{}\", but is of type \"{}\"\n\tdetails:\n{}\n{}", varname, lua_typename(lua, lua_typeid), lua_typename(lua, lua_type(lua, -1)), lua_debug_stack(), lua_debug_callstack());\
			}\
		};\
		auto ret = innerfn();\
		lua_pop(lua, stack_usage);\
		return ret;\
	}

	GET_IMPL(bool, bool, LUA_TBOOLEAN, lua_toboolean)
	GET_IMPL(int, std::int64_t, LUA_TNUMBER, lua_tointeger)
	GET_IMPL(number, double, LUA_TNUMBER, lua_tonumber)
	GET_IMPL(string, std::string, LUA_TSTRING, lua_tostring)


	#define STACK_GET_IMPL(api_typename, cpp_typename, lua_typeid, lua_convfn, lua_isfn) std::expected<cpp_typename, tz::error_code> lua_stack_get_##api_typename(std::size_t id){\
		if(lua_isfn(lua, id))\
		{\
			return lua_convfn(lua, id);\
		}\
		else\
		{\
			UNERR(tz::error_code::precondition_failure, "lua stack entry {} was requested as type \"{}\", but is of type \"{}\"\n\tdetails:\n{}\n{}", id, lua_typename(lua, lua_typeid), lua_typename(lua, lua_type(lua, id)), lua_debug_stack(), lua_debug_callstack());\
		}\
	}

	STACK_GET_IMPL(bool, bool, LUA_TBOOLEAN, lua_toboolean, lua_isboolean)
	STACK_GET_IMPL(int, std::int64_t, LUA_TNUMBER, lua_tointeger, lua_isinteger)
	STACK_GET_IMPL(number, double, LUA_TNUMBER, lua_tonumber, lua_isnumber)
	STACK_GET_IMPL(string, std::string, LUA_TSTRING, lua_tostring, lua_isstring)

	void lua_push_nil()
	{
		lua_pushnil(lua);
	}

	void lua_push_bool(bool v)
	{
		lua_pushboolean(lua, v);
	}

	void lua_push_int(std::int64_t v)
	{
		lua_pushinteger(lua, v);
	}

	void lua_push_number(double v)
	{
		lua_pushnumber(lua, v);
	}

	void lua_push_string(std::string v)
	{
		lua_pushstring(lua, v.c_str());
	}

	std::size_t lua_stack_size()
	{
		return lua_gettop(lua);
	}

	std::string lua_debug_callstack()
	{
		lua_execute("_tmp_traceback_data = debug.traceback()");
		return lua_get_string("_tmp_traceback_data").value_or("<no callstack>");
	}

	std::string lua_debug_stack()
	{
		if(!lua_checkstack(lua, 3))
		{
			return "<cant collect stack - oom>";
		}
		std::string ret;

		int top = lua_gettop(lua);
		int bottom = 1;
		ret = "=== stack (size: " + std::to_string(top - bottom + 1) + ") ===\n";
		lua_getglobal(lua, "tostring");
		for(int i = top; i >= bottom; i--)
		{
			lua_pushvalue(lua, -1);
			lua_pushvalue(lua, i);
			lua_pcall(lua, 1, 1, 0);
			const char* str = lua_tostring(lua, -1);
			ret += std::string(">") + std::to_string(i) + ": ";
			if(str == nullptr)
			{
				ret += luaL_typename(lua, i) + std::string("\n");
			}
			else
			{
				ret += str + std::string("\n");
			}
			lua_pop(lua, 1);
		}
		lua_pop(lua, 1);
		return ret + "=== end ===";
	}

	// impl

	std::vector<std::string> impl_string_split(const std::string_view& str, const std::string& delim)
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

	int impl_lua_get_var(std::string_view varname, int& stack_sz)
	{
		auto bits = impl_string_split(varname, ".");
		int type = lua_getglobal(lua, bits.front().c_str());
		if(type == LUA_TNIL)
		{
			return type;
		}
		bits.erase(bits.begin());
		stack_sz++;
		for(const std::string& bit : bits)
		{
			type = lua_getfield(lua, -1, bit.c_str());
			stack_sz++;
			if(type == LUA_TNIL)
			{
				break;
			}
		}
		return type;
	}
}