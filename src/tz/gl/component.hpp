#ifndef TOPAZ_GL2_COMPONENT_HPP
#define TOPAZ_GL2_COMPONENT_HPP

#if TZ_VULKAN
#include "tz/gl/impl/frontend/vk2/component.hpp"
#elif TZ_OGL
#include "tz/gl/impl/frontend/ogl2/component.hpp"
#endif

namespace tz::gl
{
	#if TZ_VULKAN
		using BufferComponent = BufferComponentVulkan;
		using ImageComponent = ImageComponentVulkan;
	#elif TZ_OGL
		using BufferComponent = BufferComponentOGL;
		using ImageComponent = ImageComponentOGL;
	#endif
}

#endif // TOPAZ_GL2_COMPONENT_HPP 

