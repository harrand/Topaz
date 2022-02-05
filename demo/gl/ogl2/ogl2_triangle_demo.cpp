#include "core/tz.hpp"
#include "gl/imported_shaders.hpp"
#include "gl/impl/backend/ogl2/tz_opengl.hpp"
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
		vao.bind();
		while(!tz::window().is_close_requested())
		{
			tz::window().update();
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glViewport(0, 0, static_cast<GLsizei>(tz::window().get_width()), static_cast<GLsizei>(tz::window().get_height()));
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			shader.use();
			glDrawArrays(GL_TRIANGLES, 0, 3);
		}
	}
	tz::terminate();
}
