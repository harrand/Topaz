#include "tz/tz.hpp"
#include "tz/lua/api.hpp"

// LUA BINDINGS
LUA_BEGIN(dub)
	auto v = state.stack_get_uint(1);
	v *= 2;
	state.stack_push_uint(v);
	return 1;
LUA_END

// TEST CONTENT

void register_bindings()
{
	LUA_REGISTER_ALL(dub);
}

void basic_function_tests()
{
	auto& state = tz::lua::get_state();
	state.execute("x = 2; x = dub(x); tz.assert(x == 4)");
}

int main()
{
	tz::initialise({.flags = {tz::application_flag::no_graphics, tz::application_flag::window_hidden}});
	register_bindings();

	basic_function_tests();
	tz::terminate();
}