#ifndef TOPAZ_GL2_RENDERER_HPP
#define TOPAZ_GL2_RENDERER_HPP

#if TZ_VULKAN
#include "gl/impl/frontend/vk2/renderer.hpp"
#endif

namespace tz::gl2
{
	#if TZ_VULKAN
		using Renderer = RendererVulkan;
		using RendererInfo = RendererInfoVulkan;
	#endif
}

#endif // TOPAZ_GL2_RENDERER_HPP
