#include "tz/core/tz.hpp"

int main()
{
	tz::initialise
	({
		.name = "tz_initialise_test",
		.flags = {tz::application_flag::window_hidden}
	});
	{
		// Nothing
	}
	tz::terminate();
}
