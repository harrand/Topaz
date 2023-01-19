#ifndef TZ_HPP
#define TZ_HPP

namespace tz::core
{

	/**
	 * @defgroup tz tz
	 * Low-level C++ framework utilities.
	 */

	/**
	 * @ingroup tz
	 * @defgroup tz_init Initialisation and Termination
	 * Documentation for functionality related to tz initialisation and termination.
	 */

	/**
	 * @ingroup tz
	 * @defgroup tz_job Job System
	 * Documentation for job-system.
	 */

	/**
	 * @ingroup tz
	 * @defgroup tz_memory Memory Utility
	 * Documentation for a grab-bag of memory helper structs, classes and functions.
	 */

	/**
	 * @ingroup tz_memory
	 * @defgroup tz_memory_allocator Allocators
	 * Documentation for bespoke memory allocators.
	 */

	/**
	 * @ingroup tz_init
	 * Initialise tz.
	 */
	void initialise();
	/**
	 * @ingroup tz_init
	 * Terminate tz.
	 */
	void terminate();

	namespace detail
	{
		bool is_initialised();
	}

	/*! @mainpage Home
	 * ## Introduction
	 *
	 * Welcome to the home page of the tz documentation. Here you can find detailed information about the vast majority of modules, classes and functions within tz.
	 */
}

#endif // TZ_HPP
