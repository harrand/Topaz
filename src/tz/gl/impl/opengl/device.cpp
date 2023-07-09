#if TZ_OGL
#include "tz/core/profile.hpp"
#include "tz/dbgui/dbgui.hpp"
#include "tz/gl/impl/opengl/renderer.hpp"
#include "tz/gl/impl/opengl/device.hpp"

namespace tz::gl
{
	device_render_scheduler_ogl::device_render_scheduler_ogl(const tz::gl::schedule& sched):
	sched(sched){}

	void device_render_scheduler_ogl::ogl_fingerprint(unsigned int fingerprint, std::size_t renderer_id)
	{
		this->fingerprint_to_renderer_id.emplace(fingerprint, renderer_id);
	}

	void device_render_scheduler_ogl::ogl_new_sync()
	{
		this->renderer_syncs.push_back(0);
	}

	void device_render_scheduler_ogl::ogl_gpu_do_waits(unsigned int fingerprint)
	{
		tz::assert(this->fingerprint_to_renderer_id.find(fingerprint) != this->fingerprint_to_renderer_id.end(), "fingerprint %u does not exist", fingerprint);
		std::size_t renderer_id = this->fingerprint_to_renderer_id[fingerprint];
		for(tz::gl::eid_t evt : this->sched.get_dependencies(renderer_id))
		{
			ogl_gpu_wait_on(static_cast<tz::hanval>(evt));
		}
	}

	void device_render_scheduler_ogl::ogl_gpu_wait_on(tz::gl::renderer_handle dep)
	{
		auto hval = static_cast<std::size_t>(static_cast<tz::hanval>(dep));
		tz::assert(this->renderer_syncs[hval] != 0, "trying to wait on a sync object belonging to a renderer, but the sync object was never registered, or hasnt been recognised as a renderer with dependencies.");
		glWaitSync(this->renderer_syncs[hval], 0, GL_TIMEOUT_IGNORED);
	}

	void device_render_scheduler_ogl::ogl_register_sync(unsigned int fingerprint)
	{
		tz::assert(this->fingerprint_to_renderer_id.find(fingerprint) != this->fingerprint_to_renderer_id.end(), "fingerprint %u does not exist", fingerprint);
		std::size_t hval = this->fingerprint_to_renderer_id.at(fingerprint);
		bool anyone_depends_on_me = false;
		for(tz::gl::eid_t evt : this->sched.timeline)
		{
			std::span<const tz::gl::eid_t> deps = this->sched.get_dependencies(evt);
			if(std::any_of(deps.begin(), deps.end(), [hval](tz::gl::eid_t dep){return dep == hval;}))
			{
				anyone_depends_on_me = true;
			}
		}
		if(!anyone_depends_on_me)
		{
			// if nobody depends on this renderer, its a waste of time to register a sync, so dont bother.
			return;
		}
		if(this->renderer_syncs[hval] != 0)
		{
			// kill the old sync.
			glDeleteSync(this->renderer_syncs[hval]);
		}
		this->renderer_syncs[hval] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	}

	device_ogl::device_ogl():
	device_render_scheduler_ogl(device_common<renderer_ogl>::render_graph())
	{
		glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
	}

	tz::gl::renderer_handle device_ogl::create_renderer(const renderer_info& info)
	{
		TZ_PROFZONE("OpenGL Frontend - renderer Create (via device)", 0xFFAA0000);
		device_render_scheduler_ogl::ogl_new_sync();
		tz::gl::renderer_handle ret = device_common<renderer_ogl>::emplace_renderer(info);
		device_render_scheduler_ogl::ogl_fingerprint(device_common<renderer_ogl>::get_renderer(ret).ogl_get_uid(), device_common<renderer_ogl>::renderer_count() - 1);
		return ret;
	}
	
	image_format device_ogl::get_window_format() const
	{
		return image_format::RGBA32;
	}

	void device_ogl::dbgui()
	{
		tz::gl::common_device_dbgui(*this);
	}

	void device_ogl::begin_frame()
	{

	}

	void device_ogl::end_frame()
	{

	}

	void device_ogl::full_wait() const
	{
		glFinish();
	}

	void device_ogl::frame_wait() const
	{
		tz::report("ogl frame wait NYI. doing full wait...");
		this->full_wait();
	}
}

#endif // TZ_OGL
