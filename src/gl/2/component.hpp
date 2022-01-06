#ifndef TOPAZ_GL2_COMPONENT_HPP
#define TOPAZ_GL2_COMPONENT_HPP

#if TZ_VULKAN
#include "gl/2/impl/frontend/vk2/component.hpp"
#endif

namespace tz::gl2
{
	#if TZ_VULKAN
		using BufferComponent = BufferComponentVulkan;
		using ImageComponent = ImageComponentVulkan;
	#endif
}

#endif // TOPAZ_GL2_COMPONENT_HPP 

