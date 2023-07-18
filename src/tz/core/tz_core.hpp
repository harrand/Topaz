#ifndef TZ_HPP
#define TZ_HPP

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
	 * @ingroup tz_core_memory
	 * @defgroup tz_core_memory_allocator Allocators
	 * Documentation for bespoke memory allocators.
	 */
	void initialise();
	void terminate();

	namespace detail
	{
		bool is_initialised();
	}
}

#endif // TZ_HPP
