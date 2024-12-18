#include "tz/core/lua.hpp"
#include "tz/core/job.hpp"
#include "tz/topaz.hpp"

int value = 0;
int increment()
{
	auto [val, str] = tz::lua_parse_args<int, std::string>();
	tz_assert(val == 50, "lua function passing failed arg0");
	tz_assert(str == "test", "lua function passing failed arg1");
	value++;
	return 0;
}

void do_work()
{
	tz_must(tz::lua_define_function("foo", increment));
	tz_must(tz::lua_execute("foo(50, \"test\")"));


	tz_must(tz::lua_execute("myvar = \"hello there\""));
	std::string myvar = tz_must(tz::lua_get_string("myvar"));
	tz_assert(myvar == "hello there", "string get failed");
}

#include "tz/main.hpp"
int tz_main()
{
	tz::initialise();
	// initialise topaz

	// run some lua code on main thread.
	constexpr int job_count = 69;
	// set off a bunch of jobs which all run some lua code.
	// dont care which workers end up doing it.
	do_work();
	for(std::size_t i = 0; i < job_count; i++)
	{
		tz::job_wait(tz::job_execute([]()
		{
			do_work();
		}));
	}
	tz_assert(value == job_count + 1, "boo");

	tz::terminate();
	return 0;
}