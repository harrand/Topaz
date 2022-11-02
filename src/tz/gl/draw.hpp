#ifndef TOPAZ_GL_DRAW_HPP
#define TOPAZ_GL_DRAW_HPP
#if TZ_VULKAN
#include "tz/gl/impl/backend/vk2/tz_vulkan.hpp"
#elif TZ_OGL
#include "tz/gl/impl/backend/ogl2/draw.hpp"
#endif

namespace tz::gl
{
	#if TZ_VULKAN
		using DrawIndirectCommand = VkDrawIndirectCommand;
		using DrawIndexedIndirectCommand = VkDrawIndexedIndirectCommand;
	#elif TZ_OGL
		using DrawIndirectCommand = ogl2::DrawIndirectCommand;
		using DrawIndexedIndirectCommand = ogl2::DrawIndexedIndirectCommand;
	#endif
}

#endif // TOPAZ_GL_DRAW_HPP
