#include "core/tz.hpp"
#include "gl/impl/backend/ogl2/tz_opengl.hpp"

int main()
{
	tz::GameInfo game{"ogl_initialise_test", tz::Version{1, 0, 0}, tz::info()};
	tz::initialise(game, tz::ApplicationType::HiddenWindowApplication);
	{

	}
	tz::terminate();
	return 0;

}
