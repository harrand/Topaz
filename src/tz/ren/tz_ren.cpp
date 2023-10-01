#include "tz/ren/tz_ren.hpp"
#include "tz/core/imported_text.hpp"
#include ImportedTextHeader(tz_ren, lua)

namespace tz::ren
{
	void lua_initialise(tz::lua::state& state)
	{
		state.assign_emptytable("tz.ren");

		std::string tz_ren_lua_api{ImportedTextData(tz_ren, lua)};
		state.execute(tz_ren_lua_api.data());
	}
}