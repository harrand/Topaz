#include "tz/core/tz.hpp"
#include "tz/core/window.hpp"
#include "tz/core/profiling/zone.hpp"
#include "tz/gl/device.hpp"
#include "tz/gl/renderer.hpp"
#include "tz/gl/imported_shaders.hpp"
#include "tz/dbgui/dbgui.hpp"

#include ImportedShaderHeader(tz_triangle_demo, vertex)
#include ImportedShaderHeader(tz_triangle_demo, fragment)

int main()
{
	tz::initialise
	({
		.name = "tz_triangle_demo",
	});
	{
		tz::gl::RendererInfo rinfo;
		rinfo.shader().set_shader(tz::gl::ShaderStage::Vertex, ImportedShaderSource(tz_triangle_demo, vertex));
		rinfo.shader().set_shader(tz::gl::ShaderStage::Fragment, ImportedShaderSource(tz_triangle_demo, fragment));
		rinfo.set_options({tz::gl::RendererOption::NoDepthTesting});

		tz::gl::Renderer renderer = tz::gl::device().create_renderer(rinfo);

		while(!tz::window().is_close_requested())
		{
			tz::window().begin_frame();
			renderer.render(1);
			#if TZ_DEBUG
				ImGui::ShowDemoWindow();
				ImGui::ShowStyleEditor();
			#endif // TZ_DEBUG
			tz::window().end_frame();
		}
	}
	tz::terminate();
}
