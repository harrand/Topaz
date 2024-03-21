import topaz;
import topaz.debug;

int main()
{
	tz::debug::assert_that(!tz::is_initialised(), "Real Bad: `tz::is_initialised()` returned true before `tz::initialise()` was invoked.");
	tz::initialise();
	tz::debug::assert_that(tz::is_initialised(), "Real Bad: `tz::is_initialised()` returned false between `tz::initialise()` and `tz::terminate()`.");
	tz::terminate();
	tz::debug::assert_that(!tz::is_initialised(), "Real Bad: `tz::is_initialised()` returned true after `tz::terminate()`.");
	return 0;
}
