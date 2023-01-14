#include "tz/core/tz.hpp"
#include "tz/gl/impl/opengl/detail/tz_opengl.hpp"

int main()
{
	tz::initialise
	({
		.name = "ogl_initialise_test",
		.flags = {tz::ApplicationFlag::HiddenWindow}
	});
	{

	}
	tz::terminate();
	return 0;

}
