#include "tz/tz.hpp"
#include "tz/gl/impl/opengl/detail/tz_opengl.hpp"

int main()
{
	tz::initialise
	({
		.name = "ogl_initialise_test",
		.flags = {tz::application_flag::window_hidden}
	});
	{

	}
	tz::terminate();
	return 0;

}
