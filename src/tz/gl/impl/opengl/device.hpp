#ifndef TOPAZ_GL2_IMPL_FRONTEND_OGL2_DEVICE_HPP
#define TOPAZ_GL2_IMPL_FRONTEND_OGL2_DEVICE_HPP
#if TZ_OGL
#include "tz/gl/api/device.hpp"
#include "tz/gl/declare/image_format.hpp"
#include "tz/gl/impl/common/device.hpp"
#include "tz/gl/impl/opengl/renderer.hpp"

namespace tz::gl
{
	class device_ogl : public device_common<renderer_ogl>
	{
	public:
		device_ogl();

		// Satisfies device_type.
		tz::gl::renderer_handle create_renderer(const renderer_info& info);
		using DeviceCommon<renderer_ogl>::get_renderer;
		using DeviceCommon<renderer_ogl>::destroy_renderer;
		image_format get_window_format() const;
		void dbgui();
		void begin_frame();
		void end_frame();
	private:
		std::vector<renderer_ogl> renderers;
	};
	static_assert(device_type<device_ogl, renderer_info>);
}

#endif // TZ_OGL
#endif // TOPAZ_GL2_IMPL_FRONTEND_OGL2_DEVICE_HPP
