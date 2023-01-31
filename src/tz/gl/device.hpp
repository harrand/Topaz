#ifndef TOPAZ_GL2_DEVICE_HPP
#define TOPAZ_GL2_DEVICE_HPP

#if TZ_VULKAN
#include "tz/gl/impl/vulkan/device.hpp"
#elif TZ_OGL
#include "tz/gl/impl/opengl/device.hpp"
#endif
namespace tz::gl
{
	#if TZ_VULKAN
		using device = device_vulkan;
	#elif TZ_OGL
		using device = device_ogl;
	#endif

	/**
	 * @ingroup tz_gl2
	 * Retrieve the global device. See @ref tz::gl::device_type for usage and functionality.
	 *
	 * You should not be creating your own devices.
	 */
	device& get_device();
	void destroy_device();
}

#endif // TOPAZ_GL2_DEVICE_HPP
