#ifndef TOPAZ_GL2_IMPL_FRONTEND_OGL2_DEVICE_HPP
#define TOPAZ_GL2_IMPL_FRONTEND_OGL2_DEVICE_HPP
#if TZ_OGL
#include "tz/gl/api/device.hpp"
#include "tz/gl/declare/image_format.hpp"
#include "tz/gl/impl/common/device.hpp"
#include "tz/gl/impl/opengl/renderer.hpp"

namespace tz::gl
{
	class DeviceOGL : public DeviceCommon<RendererOGL>
	{
	public:
		DeviceOGL();

		// Satisfies DeviceType.
		tz::gl::RendererHandle create_renderer(const RendererInfoOGL& info);
		using DeviceCommon<RendererOGL>::get_renderer;
		using DeviceCommon<RendererOGL>::destroy_renderer;
		ImageFormat get_window_format() const;
		void dbgui();
	private:
		std::vector<RendererOGL> renderers;
	};
	static_assert(DeviceType<DeviceOGL, RendererInfoOGL>);
}

#endif // TZ_OGL
#endif // TOPAZ_GL2_IMPL_FRONTEND_OGL2_DEVICE_HPP