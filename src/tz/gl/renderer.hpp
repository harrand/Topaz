#ifndef TOPAZ_GL2_RENDERER_HPP
#define TOPAZ_GL2_RENDERER_HPP

#if TZ_VULKAN
#include "tz/gl/impl/vulkan/renderer.hpp"
#include "tz/gl/impl/vulkan/renderer2.hpp"
#elif TZ_OGL
#include "tz/gl/impl/opengl/renderer.hpp"
#endif

namespace tz::gl
{
	#if TZ_VULKAN
		using renderer = renderer_vulkan;
		using renderer2 = renderer_vulkan2;
	#elif TZ_OGL
		using renderer = renderer_ogl;
	#endif
}

#endif // TOPAZ_GL2_RENDERER_HPP
