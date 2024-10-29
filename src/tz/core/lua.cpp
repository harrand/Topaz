#include "tz/core/lua.hpp"
#include "tz/topaz.hpp"

#include "tz/core/job.hpp"

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
			RETERR(tz::error_code::unknown_error, "lua error while executing file {}: ", path.filename().string(), err);
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
			RETERR(tz::error_code::unknown_error, "lua error while executing code \"{}...\": {}", code_snippet.c_str(), err != nullptr ? err : "<no error message>");
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

	tz::error_code lua_define_function(std::string_view varname, lua_fn fn)
	{
		lua_pushcfunction(lua, reinterpret_cast<lua_CFunction>(fn));
		std::string tmp = std::format("tmp_{}", varname);
		lua_setglobal(lua, tmp.c_str());
		lua_execute(std::format("{} = {}", varname, tmp));
		return tz::error_code::success;
	}
}