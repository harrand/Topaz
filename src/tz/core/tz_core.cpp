#include "tz/tz.hpp"
#include "tz/core/debug.hpp"
#include "tz/core/job/job.hpp"

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

	namespace detail
	{
		bool is_initialised()
		{
			return detail::init.initialised;
		}
	}
}
