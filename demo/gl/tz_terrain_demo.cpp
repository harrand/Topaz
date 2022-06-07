#include "core/tz.hpp"
#include "core/window.hpp"
#include "core/profiling/zone.hpp"
#include "gl/device.hpp"
#include "gl/renderer.hpp"
#include "gl/imported_shaders.hpp"

#include ImportedShaderHeader(tz_terrain_demo, vertex)
#include ImportedShaderHeader(tz_terrain_demo, fragment)

int main()
{
	tz::initialise
	({
		.name = "tz_terrain_demo",
	});
	{
		tz::gl::Device dev;

		tz::gl::RendererInfo rinfo;
		rinfo.shader().set_shader(tz::gl::ShaderStage::Vertex, ImportedShaderSource(tz_terrain_demo, vertex));
		rinfo.shader().set_shader(tz::gl::ShaderStage::Fragment, ImportedShaderSource(tz_terrain_demo, fragment));

		tz::gl::Renderer renderer = dev.create_renderer(rinfo);

		while(!tz::window().is_close_requested())
		{
			TZ_FRAME_BEGIN;
			tz::window().update();
			renderer.render(2);
			TZ_FRAME_END;
		}
	}
	tz::terminate();
}
