#include "tz/core/tz.hpp"

int main()
{
	tz::initialise
	({
		.name = "tz_initialise_test",
		.flags = {tz::ApplicationFlag::HiddenWindow}
	});
	{
		// Nothing
	}
	tz::terminate();
}
