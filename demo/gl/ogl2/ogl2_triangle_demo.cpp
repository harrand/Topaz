#include "tz/tz.hpp"
#include "tz/gl/imported_shaders.hpp"
#include "tz/gl/impl/opengl/detail/tz_opengl.hpp"
#include "tz/gl/impl/opengl/detail/framebuffer.hpp"
#include "tz/gl/impl/opengl/detail/shader.hpp"
#include "tz/gl/impl/opengl/detail/vertex_array.hpp"

#include ImportedShaderHeader(triangle_demo, vertex)
#include ImportedShaderHeader(triangle_demo, fragment)

int main()
{
	tz::initialise
	({
		.name = "ogl2_triangle_demo",
	});
	{
		using namespace tz::gl::ogl2;
		VertexArray vao;

		OGLString vtx_src = std::string(ImportedShaderSource(triangle_demo, vertex));
		OGLString frg_src = std::string(ImportedShaderSource(triangle_demo, fragment));

		Shader shader
		{{
			.modules =
			{
				{
					.type = ShaderType::vertex,
					.code = vtx_src
				},
				{
					.type = ShaderType::fragment,
					.code = frg_src
				}
			 }
		}};
		Framebuffer frame = Framebuffer::null();
		while(!tz::window().is_close_requested())
		{
			tz::window().update();
			frame.bind();
			frame.clear();
			shader.use();
			vao.draw(1);
		}
	}
	tz::terminate();
}
