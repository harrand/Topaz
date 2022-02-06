#include "core/tz.hpp"
#include "gl/impl/backend/ogl2/tz_opengl.hpp"

int main()
{
	tz::initialise
	({
		.name = "ogl_initialise_test",
		.app_type = tz::ApplicationType::HiddenWindowApplication
	});
	{

	}
	tz::terminate();
	return 0;

}
