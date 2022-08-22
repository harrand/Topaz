#ifndef TOPAZ_GL2_DEVICE_HPP
#define TOPAZ_GL2_DEVICE_HPP

#if TZ_VULKAN
#include "tz/gl/impl/frontend/vk2/device.hpp"
#elif TZ_OGL
#include "tz/gl/impl/frontend/ogl2/device.hpp"
#endif
namespace tz::gl
{
	#if TZ_VULKAN
		using Device = DeviceVulkan;
	#elif TZ_OGL
		using Device = DeviceOGL;
	#endif

	/**
	 * @ingroup tz_gl2
	 * Retrieve the global device. See @ref tz::gl::DeviceType for usage and functionality.
	 *
	 * You should not be creating your own devices.
	 */
	Device& device();
	void destroy_device();
}

#endif // TOPAZ_GL2_DEVICE_HPP
