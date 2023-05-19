#ifndef TOPAZ_GL2_IMPL_FRONTEND_OGL2_DEVICE_HPP
#define TOPAZ_GL2_IMPL_FRONTEND_OGL2_DEVICE_HPP
#if TZ_OGL
#include "tz/gl/api/device.hpp"
#include "tz/gl/declare/image_format.hpp"
#include "tz/gl/impl/opengl/renderer.hpp"
#include <unordered_map>

namespace tz::gl
{
	class device_render_scheduler_ogl
	{
	public:
		device_render_scheduler_ogl(const tz::gl::schedule& sched);
		void ogl_fingerprint(unsigned int fingerprint, std::size_t renderer_id);
		void ogl_gpu_do_waits(unsigned int fingerprint);
		void ogl_gpu_wait_on(tz::gl::renderer_handle dep);
		void ogl_register_sync(unsigned int fingerprint);
		void ogl_new_sync();
	private:
		std::vector<GLsync> renderer_syncs = {};
		std::unordered_map<unsigned int, std::size_t> fingerprint_to_renderer_id = {};
		const tz::gl::schedule& sched;
	};

	class device_ogl : public device_render_scheduler_ogl, public device_common<renderer_ogl>
	{
	public:
		device_ogl();

		// Satisfies device_type.
		tz::gl::renderer_handle create_renderer(const renderer_info& info);
		using device_common<renderer_ogl>::get_renderer;
		using device_common<renderer_ogl>::destroy_renderer;
		image_format get_window_format() const;
		void dbgui();
		void begin_frame();
		void end_frame();
	};
	static_assert(device_type<device_ogl, renderer_info>);
}

#endif // TZ_OGL
#endif // TOPAZ_GL2_IMPL_FRONTEND_OGL2_DEVICE_HPP
