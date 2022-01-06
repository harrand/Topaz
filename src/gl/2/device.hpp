#ifndef TOPAZ_GL2_DEVICE_HPP
#define TOPAZ_GL2_DEVICE_HPP

#if TZ_VULKAN
#include "gl/2/impl/frontend/vk2/device.hpp"
#endif
namespace tz::gl2
{
	#if TZ_VULKAN
		using Device = DeviceVulkan;
	#endif
}

#endif // TOPAZ_GL2_DEVICE_HPP
