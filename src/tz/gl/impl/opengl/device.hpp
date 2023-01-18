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

		// Satisfies device_type.
		tz::gl::renderer_handle create_renderer(const renderer_infoOGL& info);
		using DeviceCommon<RendererOGL>::get_renderer;
		using DeviceCommon<RendererOGL>::destroy_renderer;
		image_format get_window_format() const;
		void dbgui();
	private:
		std::vector<RendererOGL> renderers;
	};
	static_assert(device_type<DeviceOGL, renderer_infoOGL>);
}

#endif // TZ_OGL
#endif // TOPAZ_GL2_IMPL_FRONTEND_OGL2_DEVICE_HPP
