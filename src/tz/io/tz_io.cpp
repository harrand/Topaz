#include "tz/io/tz_io.hpp"
#include "tz/core/imported_text.hpp"
#include ImportedTextHeader(tz_io, lua)

namespace tz::io
{
	void lua_initialise(tz::lua::state& state)
	{
		state.assign_emptytable("tz.io");

		std::string tz_io_lua_api{ImportedTextData(tz_io, lua)};
		state.execute(tz_io_lua_api.data());
	}
}