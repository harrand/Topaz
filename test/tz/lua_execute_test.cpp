#include "tz/core/lua.hpp"
#include "tz/core/job.hpp"
#include "tz/topaz.hpp"

int value = 0;
int increment(void* ctx)
{
	(void)ctx;
	value++;
	return 0;
}

void do_work()
{
	tz_must(tz::lua_define_function("foo", increment));
	tz_must(tz::lua_execute("foo()"));
}

int main()
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