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

struct foo
{
	foo(int x):
	x(x)
	{
	}
	int bar(tz::lua::state& state)
	{
		x++;
		return 0;
	}
	int x = 0;
	char y = 'E';
};

LUA_CLASS_BEGIN(foo)
	LUA_CLASS_METHODS_BEGIN
		LUA_METHOD(foo, bar)
	LUA_CLASS_METHODS_END
LUA_CLASS_END

LUA_BEGIN(make_foo)
	auto[initial_value] = tz::lua::parse_args<int>(state);
	LUA_CLASS_PUSH(state, foo, {initial_value});
	return 1;
LUA_END

LUA_BEGIN(foo_increment)
	foo& f = state.stack_get_userdata<foo>(1);
	f.x++;
	return 0;
LUA_END

LUA_BEGIN(get_foo)
	foo& f = state.stack_get_userdata<foo>(1);
	state.stack_push_int(f.x);
	return 1;
LUA_END

void register_bindings()
{
	LUA_REGISTER_ALL(dub);
	LUA_REGISTER_ALL(make_foo);
	LUA_REGISTER_ALL(foo_increment);
	LUA_REGISTER_ALL(get_foo);
	LUA_NAMESPACE_REGISTER_ALL(my_poggers_lib);
	LUA_CLASS_REGISTER_ALL(foo);
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
	state.execute(R"(
		foo1 = make_foo(1);
		tz.assert(get_foo(foo1) == 1)
		foo2 = make_foo(1);
		tz.assert(get_foo(foo2) == 1)
		foo1:bar();
		tz.assert(get_foo(foo1) == 2)
		tz.assert(get_foo(foo2) == 1)
	)");
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