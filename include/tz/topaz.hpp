#ifndef TOPAZ_HPP
#define TOPAZ_HPP

/**
 * @defgroup topaz Topaz Engine
 * Top-level engine functionality.
 * @{
**/
namespace tz
{
	/**
	 * @ingroup topaz
	 * Initialise the engine.
	**/
	void initialise();
	/**
	 * @ingroup topaz
	 * Terminate the engine, cleaning up all resources.
	**/
	void terminate();
}

#include "detail/debug.hpp"

/**
 * @}
**/

#endif // TOPAZ_HPP