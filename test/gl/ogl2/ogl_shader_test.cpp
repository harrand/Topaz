#include "core/tz.hpp"
#include "gl/impl/backend/ogl2/tz_opengl.hpp"
#include "gl/impl/backend/ogl2/shader.hpp"

void create_empty_shader()
{
	using namespace tz::gl::ogl2;
	Shader shader
	{{
		.modules =
		{
			{
				.type = ShaderType::Vertex,
				.code = "#version 430\nvoid main(){}"
			},
			{
				.type = ShaderType::Fragment,
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
		.app_type = tz::ApplicationType::HiddenWindowApplication
	});
	{
		create_empty_shader();
	}
	tz::terminate();
	return 0;

}
