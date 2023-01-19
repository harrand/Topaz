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
	 * @class renderer
	 * Implementation of @ref tz::gl::renderer_type. See concept for more info.
	 *
	 * Ideally there are no implementation details to concern yourself with. However, the implementations for each graphics API are implemented:
	 * - Vulkan: @ref tz::gl::renderer_vulkan
	 * - OGL: @ref tz::gl::renderer_ogl
	 */
	#if TZ_VULKAN
		using renderer = renderer_vulkan;
		using renderer_info = renderer_info_vulkan;
	#elif TZ_OGL
		using renderer = renderer_ogl;
		using renderer_info = renderer_info_ogl;
	#endif
}

#endif // TOPAZ_GL2_RENDERER_HPP
