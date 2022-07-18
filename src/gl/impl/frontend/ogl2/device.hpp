#ifndef TOPAZ_GL2_IMPL_FRONTEND_OGL2_DEVICE_HPP
#define TOPAZ_GL2_IMPL_FRONTEND_OGL2_DEVICE_HPP
#if TZ_OGL
#include "gl/api/device.hpp"
#include "gl/declare/image_format.hpp"
#include "gl/impl/frontend/ogl2/renderer.hpp"

namespace tz::gl
{
	class DeviceOGL
	{
	public:
		DeviceOGL();

		// Satisfies DeviceType.
		RendererOGL create_renderer(const RendererInfoOGL& info);
		ImageFormat get_window_format() const;
	};
	static_assert(DeviceType<DeviceOGL, RendererInfoOGL>);
}

#endif // TZ_OGL
#endif // TOPAZ_GL2_IMPL_FRONTEND_OGL2_DEVICE_HPP
