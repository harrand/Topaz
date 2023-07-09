#ifndef TOPAZ_GL_IMPL_BACKEND_OGL2_DRAW_HPP
#define TOPAZ_GL_IMPL_BACKEND_OGL2_DRAW_HPP
#if TZ_OGL
#include <cstddef>

namespace tz::gl::ogl2
{
	struct draw_indirect_command
	{
		std::uint32_t count;
		std::uint32_t instance_count = 1;
		std::uint32_t first;
		std::uint32_t base_instance = 0;
	};

	struct draw_indexed_indirect_command
	{
		std::uint32_t count;
		std::uint32_t instance_count = 1;
		std::uint32_t first_index;
		std::int32_t base_vertex = 0;
		std::uint32_t base_instance = 0;
	};
}

#endif // TZ_OGL
#endif // TOPAZ_GL_IMPL_BACKEND_OGL2_DRAW_HPP
