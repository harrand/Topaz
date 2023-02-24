#include "tz/tz.hpp"
#include "tz/gl/impl/opengl/detail/tz_opengl.hpp"
#include "tz/gl/impl/opengl/detail/shader.hpp"

void create_empty_shader()
{
	using namespace tz::gl::ogl2;
	shader shader
	{{
		.modules =
		{
			{
				.type = shader_type::vertex,
				.code = "#version 430\nvoid main(){}"
			},
			{
				.type = shader_type::fragment,
				.code = "#version 430\nvoid main(){}"
			}
		}
	}};
}

int main()
{
	tz::initialise
	({
		.name = "ogl_shader_test",
		.flags = {tz::application_flag::window_hidden}
	});
	{
		create_empty_shader();
	}
	tz::terminate();
	return 0;

}
