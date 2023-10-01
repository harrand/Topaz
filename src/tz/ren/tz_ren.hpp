#ifndef TOPAZ_REN_TZ_REN_HPP
#define TOPAZ_REN_TZ_REN_HPP
#include "tz/lua/api.hpp"

namespace tz::ren
{
	/**
	 * @ingroup tz_cpp
	 * @defgroup tz_ren Rendering Library
	 * High-level 3D rendering library. Built ontop of @ref tz_gl2.
	 *
	 * The rendering library is mostly comprised of high-level renderer classes.
	 */

	 void lua_initialise(tz::lua::state& state);
}

#endif // TOPAZ_REN_TZ_REN_HPP