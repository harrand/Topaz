#ifndef TOPAZ_GL2_DEVICE_HPP
#define TOPAZ_GL2_DEVICE_HPP

#if TZ_VULKAN
#include "tz/gl/impl/vulkan/device.hpp"
#include "tz/gl/impl/vulkan/device2.hpp"
#elif TZ_OGL
#include "tz/gl/impl/opengl/device.hpp"
#endif
namespace tz::gl
{
	#if TZ_VULKAN
		using device = device_vulkan;
		using device2 = device_vulkan2;
	#elif TZ_OGL
		using device = device_ogl;
		using device2 = device_ogl;
	#endif

	/**
	 * @ingroup tz_gl2
	 * Retrieve the global device. See @ref tz::gl::device_type for usage and functionality.
	 *
	 * You should not be creating your own devices.
	 */
	device& get_device();
	device2& get_device2();
	void destroy_device();
}

#endif // TOPAZ_GL2_DEVICE_HPP
