#ifndef TOPAZ_GL2_RENDERER_HPP
#define TOPAZ_GL2_RENDERER_HPP

#if TZ_VULKAN
#include "tz/gl/impl/vulkan/renderer.hpp"
#elif TZ_OGL
#include "tz/gl/impl/opengl/renderer.hpp"
#endif

namespace tz::gl
{

	/**
	 * @ingroup tz_gl2_renderer
	 * @class Renderer
	 * Implementation of @ref tz::gl::renderer_type. See concept for more info.
	 *
	 * Ideally there are no implementation details to concern yourself with. However, the implementations for each graphics API are implemented:
	 * - Vulkan: @ref tz::gl::RendererVulkan
	 * - OGL: @ref tz::gl::RendererOGL
	 */
	#if TZ_VULKAN
		using Renderer = RendererVulkan;
		using renderer_info = renderer_infoVulkan;
	#elif TZ_OGL
		using Renderer = RendererOGL;
		using renderer_info = renderer_infoOGL;
	#endif
}

#endif // TOPAZ_GL2_RENDERER_HPP
