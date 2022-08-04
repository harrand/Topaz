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
		tz::gl::Device dev;
		tz::dbgui::initialise({.device = &dev});
		{

			tz::gl::RendererInfo rinfo;
			rinfo.shader().set_shader(tz::gl::ShaderStage::Vertex, ImportedShaderSource(tz_triangle_demo, vertex));
			rinfo.shader().set_shader(tz::gl::ShaderStage::Fragment, ImportedShaderSource(tz_triangle_demo, fragment));
			rinfo.set_options({tz::gl::RendererOption::NoDepthTesting, tz::gl::RendererOption::NoPresent});

			tz::gl::Renderer renderer = dev.create_renderer(rinfo);

			while(!tz::window().is_close_requested())
			{
				TZ_FRAME_BEGIN;
				tz::window().update();
				tz::dbgui::begin_frame();
				renderer.render(1);
				ImGui::Begin("Well met", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
				ImGui::Text("i am harrand, gee i hope this text works!");
				ImGui::Button("press me hnngh");
				ImGui::End();
				tz::dbgui::end_frame();
				TZ_FRAME_END;
			}
		}
		tz::dbgui::terminate();
	}
	tz::terminate();
}
