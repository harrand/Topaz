#include "tz/core/tz.hpp"
#include "tz/gl/impl/backend/ogl2/tz_opengl.hpp"

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