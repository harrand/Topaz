#ifndef TOPAZ_HPP
#define TOPAZ_HPP

/**
 * @defgroup tz Topaz API Reference
**/
namespace tz
{
	/**
	 * @ingroup tz
	 * @brief Initialise the engine.
	 *
	 * You should call this at the start of your program's runtime, before the first Topaz API call you make. You should expect this function to take a significantly long time.
	**/
	void initialise();
	/**
	 * @ingroup tz
	 * @brief Terminate the engine, cleaning up all resources.
	 *
	 * You should call this at the end of your program's runtime, after the last Topaz API call you make. You should expect this function to take a significantly long time.
	**/
	void terminate();
}

/**
 * @ingroup tz
 * @defgroup tz_core Core Libraries
 * @brief Lowest-level libraries, structs and functionality. Everything is expected to depend on this.
**/

#include "detail/debug.hpp"

#endif // TOPAZ_HPP