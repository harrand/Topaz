#include "core/tz.hpp"
#include "gl/impl/backend/ogl2/tz_opengl.hpp"
#include "gl/impl/backend/ogl2/buffer.hpp"

void create_empty_buffer()
{
	using namespace tz::gl::ogl2;
	Buffer static_buf
	{{
		.target = BufferTarget::Uniform,
		.residency = BufferResidency::Static
	}};
}

int main()
{
	tz::GameInfo game{"ogl_buffer_test", tz::Version{1, 0, 0}, tz::info()};
	tz::initialise(game, tz::ApplicationType::HiddenWindowApplication);
	{
		create_empty_buffer();
	}
	tz::terminate();
	return 0;
}
