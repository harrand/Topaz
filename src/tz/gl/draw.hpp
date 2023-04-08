#ifndef TOPAZ_GL_DRAW_HPP
#define TOPAZ_GL_DRAW_HPP
#if TZ_VULKAN
#include "tz/gl/impl/vulkan/detail/draw.hpp"
#elif TZ_OGL
#include "tz/gl/impl/opengl/detail/draw.hpp"
#endif

namespace tz::gl
{
	#if TZ_VULKAN
		using draw_indirect_command = vk2::draw_indirect_command;
		using draw_indexed_indirect_command = vk2::draw_indexed_indirect_command;
	#elif TZ_OGL
		using draw_indirect_command = ogl2::draw_indirect_command;
		using draw_indexed_indirect_command = ogl2::draw_indexed_indirect_command;
	#endif
}

#endif // TOPAZ_GL_DRAW_HPP
