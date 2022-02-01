#ifndef TOPAZ_GL2_RENDERER_HPP
#define TOPAZ_GL2_RENDERER_HPP

#if TZ_VULKAN
#include "gl/impl/frontend/vk2/renderer.hpp"
#endif

namespace tz::gl2
{

	/**
	 * @ingroup tz_gl2_renderer
	 * @class Renderer
	 * Implementation of @ref tz::gl2::RendererType. See concept for more info.
	 *
	 * Ideally there are no implementation details to concern yourself with. However, the implementations for each graphics API are implemented:
	 * - Vulkan: @ref tz::gl2::RendererVulkan
	 * - OGL: Not Yet Implemented
	 */
	#if TZ_VULKAN
		using Renderer = RendererVulkan;
		using RendererInfo = RendererInfoVulkan;
	#endif
}

#endif // TOPAZ_GL2_RENDERER_HPP
