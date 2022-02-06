#include "core/tz.hpp"

int main()
{
	tz::initialise
	({
		.name = "tz_initialise_test",
		.app_type = tz::ApplicationType::HiddenWindowApplication
	});
	{
		// Nothing
	}
	tz::terminate();
}
