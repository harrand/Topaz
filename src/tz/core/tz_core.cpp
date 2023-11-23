#include "tz/core/tz_core.hpp"
#include "tz/tz.hpp"
#include "tz/core/debug.hpp"
#include "tz/core/job/job.hpp"

#include "tz/core/time.hpp"
#include "tz/core/data/data_store.hpp"

#include "tz/core/imported_text.hpp"
#include ImportedTextHeader(tz_core, lua)

#undef assert

namespace tz::core
{
	namespace detail
	{
		struct init_state
		{
			bool initialised = false;
		};

		static init_state init = {};
	}

	void initialise()
	{
		tz::assert(!detail::init.initialised, "initialise() already initialised");
		tz::detail::job_system_init();
		detail::init.initialised = true;
	}

	void terminate()
	{
		tz::assert(detail::init.initialised, "terminate() called but we are not initialised");
		tz::detail::job_system_term();
		detail::init.initialised = false;
	}

	LUA_BEGIN(impl_tz_system_time)
		state.stack_push_uint(tz::system_time().millis<std::uint64_t>());
		return 1;
	LUA_END

	void lua_initialise(tz::lua::state& state)
	{
		state.assign_func("impl_tz_system_time", LUA_FN_NAME(impl_tz_system_time));
		state.execute("tz.time = impl_tz_system_time");

		state.new_type("tz_lua_data_store", LUA_CLASS_NAME(tz_lua_data_store)::registers);

		std::string tz_core_lua_api{ImportedTextData(tz_core, lua)};
		state.execute(tz_core_lua_api.data());
	}

	namespace detail
	{
		bool is_initialised()
		{
			return detail::init.initialised;
		}
	}
}
