#ifndef TOPAZ_GL2_COMPONENT_HPP
#define TOPAZ_GL2_COMPONENT_HPP

#if TZ_VULKAN
#include "tz/gl/impl/vulkan/component.hpp"
#elif TZ_OGL
#include "tz/gl/impl/opengl/component.hpp"
#endif

namespace tz::gl
{
	#if TZ_VULKAN
		using buffer_component = buffer_component_vulkan;
		using image_component = image_component_vulkan;
	#elif TZ_OGL
		using buffer_component = buffer_component_ogl;
		using image_component = image_component_ogl;
	#endif
}

#endif // TOPAZ_GL2_COMPONENT_HPP 

