#ifndef TZ_HPP
#define TZ_HPP
#include "tz/lua/api.hpp"

namespace tz::core
{

	/**
	 * @ingroup tz_core
	 * @defgroup tz_core_job Job System
	 * Documentation for job-system.
	 */

	/**
	 * @ingroup tz_core
	 * @defgroup tz_core_memory Memory Utility
	 * Documentation for a grab-bag of memory helper structs, classes and functions.
	 */

	/**
	 * @ingroup tz_core
	 * @defgroup tz_core_messaging Messaging
	 * Documentation for object-to-object messaging.
	 */

	/**
	 * @ingroup tz_core_memory
	 * @defgroup tz_core_memory_allocator Allocators
	 * Documentation for bespoke memory allocators.
	 */
	void initialise();
	void terminate();
	void lua_initialise(tz::lua::state& state);

	namespace detail
	{
		bool is_initialised();
	}
}

#endif // TZ_HPP
