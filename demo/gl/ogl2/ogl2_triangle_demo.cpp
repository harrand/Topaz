#include "core/tz.hpp"
#include "gl/imported_shaders.hpp"
#include "gl/impl/backend/ogl2/tz_opengl.hpp"
#include "gl/impl/backend/ogl2/framebuffer.hpp"
#include "gl/impl/backend/ogl2/shader.hpp"
#include "gl/impl/backend/ogl2/vertex_array.hpp"

#include ImportedShaderHeader(triangle_demo, vertex)
#include ImportedShaderHeader(triangle_demo, fragment)

int main()
{
	tz::initialise
	({
		.name = "ogl2_triangle_demo",
		.window = {}
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
					.type = ShaderType::Vertex,
					.code = vtx_src
				},
				{
					.type = ShaderType::Fragment,
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
