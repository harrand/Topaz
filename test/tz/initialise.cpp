import topaz;
import topaz.debug;
import topaz.os;
#include <iostream>


int main()
{
	tz::debug::assert_that(!tz::is_initialised(), "Real Bad: `tz::is_initialised()` returned true before `tz::initialise()` was invoked.");
	tz::initialise();
	tz::debug::assert_that(tz::is_initialised(), "Real Bad: `tz::is_initialised()` returned false between `tz::initialise()` and `tz::terminate()`.");
	tz::terminate();
	tz::debug::assert_that(!tz::is_initialised(), "Real Bad: `tz::is_initialised()` returned true after `tz::terminate()`.");

	tz::os::system sys = tz::os::get_system();
	std::cout << "system: " << sys.to_string() << "\n";

	return 0;
}
