#include "tz/core/lua.hpp"
#include "tz/core/job.hpp"
#include "tz/topaz.hpp"

int main()
{
	tz::initialise();
	// initialise topaz

	// run some lua code on main thread.
	tz_must(tz::lua_execute("print(42069)"));
	// set off a bunch of jobs which all run some lua code.
	// dont care which workers end up doing it.
	for(std::size_t i = 0; i < 69; i++)
	{
		tz::job_wait(tz::job_execute([]()
		{
			tz_must(tz::lua_execute("print(123)"));
		}));
	}
	tz::terminate();
	return 0;
}