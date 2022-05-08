#include "core/tz.hpp"
#include "core/window.hpp"
#include "core/profiling/zone.hpp"
#include "gl/device.hpp"
#include "gl/renderer.hpp"
#include "gl/imported_shaders.hpp"
#include "gl/resource.hpp"

#include ImportedShaderHeader(tz_compute_demo, compute)

int main()
{
	tz::initialise
	({
		.name = "tz_compute_demo",
	});
	{
		tz::gl::Device dev;

		tz::gl::BufferResource colour_buffer = tz::gl::BufferResource::from_many
		({
			tz::Vec4{0.0f, 0.0f, 0.0f, 1.0f},
			tz::Vec4{1.0f, 0.0f, 0.0f, 1.0f},
			tz::Vec4{0.0f, 1.0f, 0.0f, 1.0f},
			tz::Vec4{0.0f, 0.0f, 1.0f, 1.0f}
		});

		tz::gl::RendererInfo pinfo;
		pinfo.shader().set_shader(tz::gl::ShaderStage::Compute, ImportedShaderSource(tz_compute_demo, compute));
		pinfo.set_options({tz::gl::RendererOption::BlockingCompute});
		auto cbuf = pinfo.add_resource(colour_buffer);

		tz::gl::Renderer compute_worker = dev.create_renderer(pinfo);

		while(!tz::window().is_close_requested())
		{
			TZ_FRAME_BEGIN;
			tz::window().update();
			compute_worker.render(1);
			TZ_FRAME_END;
		}
	}
	tz::terminate();
}
