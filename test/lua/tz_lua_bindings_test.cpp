#include "tz/tz.hpp"
#include "tz/lua/api.hpp"

// LUA BINDINGS
LUA_BEGIN(dub)
	auto v = state.stack_get_uint(1);
	v *= 2;
	state.stack_push_uint(v);
	return 1;
LUA_END

LUA_NAMESPACE_BEGIN(my_poggers_lib)
	LUA_NAMESPACE_FUNC_BEGIN(yee)
		const auto[par1, par2, par3] = tz::lua::parse_args<bool, std::string, unsigned int>(state);
		tz::assert(!par1);
		tz::assert(par2 == "hunter2");
		tz::assert(par3 == 42);
		state.stack_push_uint(123);
		return 1;
	LUA_NAMESPACE_FUNC_END
LUA_NAMESPACE_END

// TEST CONTENT

void register_bindings()
{
	LUA_REGISTER_ALL(dub);
	LUA_NAMESPACE_REGISTER_ALL(my_poggers_lib);
}

void basic_function_tests()
{
	auto& state = tz::lua::get_state();
	state.execute("x = 2; x = dub(x); tz.assert(x == 4)");
}

void basic_namespace_tests()
{
	auto& state = tz::lua::get_state();
	state.execute("answer = my_poggers_lib.yee(false, \"hunter2\", 42); tz.assert(answer == 123)");
}

void basic_class_tests()
{
	auto& state = tz::lua::get_state();
	//state.execute("holder.test()");
}

int main()
{
	tz::initialise({.flags = {tz::application_flag::no_graphics, tz::application_flag::window_hidden}});
	register_bindings();

	basic_function_tests();
	basic_namespace_tests();
	basic_class_tests();
	tz::terminate();
}