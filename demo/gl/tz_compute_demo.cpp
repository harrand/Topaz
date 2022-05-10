#include "core/tz.hpp"
#include "core/window.hpp"
#include "core/profiling/zone.hpp"
#include "gl/device.hpp"
#include "gl/renderer.hpp"
#include "gl/imported_shaders.hpp"
#include "gl/resource.hpp"

#include ImportedShaderHeader(tz_compute_demo, compute)
#include ImportedShaderHeader(tz_compute_demo_render, vertex)
#include ImportedShaderHeader(tz_compute_demo_render, fragment)

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
		tz::gl::BufferResource time_buffer = tz::gl::BufferResource::from_one(0u);

		tz::gl::RendererInfo pinfo;
		pinfo.shader().set_shader(tz::gl::ShaderStage::Compute, ImportedShaderSource(tz_compute_demo, compute));
		auto cbuf = pinfo.add_resource(colour_buffer);
		pinfo.add_resource(time_buffer);

		tz::gl::Renderer compute_worker = dev.create_renderer(pinfo);

		tz::gl::RendererInfo rinfo;
		rinfo.shader().set_shader(tz::gl::ShaderStage::Vertex, ImportedShaderSource(tz_compute_demo_render, vertex));
		rinfo.shader().set_shader(tz::gl::ShaderStage::Fragment, ImportedShaderSource(tz_compute_demo_render, fragment));
		auto refbuf = rinfo.add_component(*compute_worker.get_component(cbuf));
		tz::gl::Renderer renderer = dev.create_renderer(rinfo);

		while(!tz::window().is_close_requested())
		{
			TZ_FRAME_BEGIN;
			tz::window().update();
			compute_worker.render();
			renderer.render(1);
			TZ_FRAME_END;
		}
	}
	tz::terminate();
}
