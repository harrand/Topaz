#include "tz/core/tz.hpp"
#include "tz/gl/impl/opengl/detail/tz_opengl.hpp"
#include "tz/gl/impl/opengl/detail/shader.hpp"

void create_empty_shader()
{
	using namespace tz::gl::ogl2;
	Shader shader
	{{
		.modules =
		{
			{
				.type = ShaderType::vertex,
				.code = "#version 430\nvoid main(){}"
			},
			{
				.type = ShaderType::fragment,
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
		.flags = {tz::application_flag::HiddenWindow}
	});
	{
		create_empty_shader();
	}
	tz::terminate();
	return 0;

}
