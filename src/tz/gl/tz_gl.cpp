#include "tz/gl/tz_gl.hpp"
#include "tz/gl/device.hpp"

namespace tz::gl
{

	void lua_initialise(tz::lua::state& state)
	{
		state.assign_emptytable("tz.gl");
		lua_initialise_device(state);
	}
}