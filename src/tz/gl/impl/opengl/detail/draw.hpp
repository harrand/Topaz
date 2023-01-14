#ifndef TOPAZ_GL_IMPL_BACKEND_OGL2_DRAW_HPP
#define TOPAZ_GL_IMPL_BACKEND_OGL2_DRAW_HPP
#if TZ_OGL

namespace tz::gl::ogl2
{
	struct DrawIndirectCommand
	{
		unsigned int count;
		unsigned int instance_count = 1;
		unsigned int first;
		unsigned int base_instance = 0;
	};

	struct DrawIndexedIndirectCommand
	{
		unsigned int count;
		unsigned int instance_count = 1;
		unsigned int first_index;
		int base_vertex = 0;
		unsigned int base_instance = 0;
	};
}

#endif // TZ_OGL
#endif // TOPAZ_GL_IMPL_BACKEND_OGL2_DRAW_HPP
