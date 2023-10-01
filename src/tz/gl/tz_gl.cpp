#include "tz/gl/tz_gl.hpp"
#include "tz/gl/device.hpp"
#include "tz/core/imported_text.hpp"
#include ImportedTextHeader(tz_gl, lua)

namespace tz::gl
{

	void lua_initialise(tz::lua::state& state)
	{
		state.assign_emptytable("tz.gl");
		lua_initialise_device(state);

		std::string tz_gl_lua_api{ImportedTextData(tz_gl, lua)};
		state.execute(tz_gl_lua_api.data());
	}
}