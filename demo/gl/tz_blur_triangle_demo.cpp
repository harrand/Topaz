#include "tz/tz.hpp"
#include "tz/core/profile.hpp"
#include "tz/gl/device.hpp"
#include "tz/gl/renderer.hpp"
#include "tz/gl/resource.hpp"
#include "tz/gl/output.hpp"
#include "tz/gl/imported_shaders.hpp"

#include ImportedShaderHeader(blur, vertex)
#include ImportedShaderHeader(blur, fragment)
#include ImportedShaderHeader(tz_triangle_demo, vertex)
#include ImportedShaderHeader(tz_triangle_demo, fragment)

int main()
{
	tz::initialise
	({
		.name = "tz_blur_triangle_demo",
		.flags = {tz::application_flag::window_noresize}
	});
	{
		// Create a renderer which just draws a square with a texture applied (fragment-shader post-processing does a blur)
		struct BlurData
		{
			tz::vec2 direction;
			float pad0[2];
		};

		tz::gl::buffer_resource blur_data = tz::gl::buffer_resource::from_one(BlurData{},
		{
			.access = tz::gl::resource_access::dynamic_fixed
		});
		tz::gl::image_resource blur_image = tz::gl::image_resource::from_uninitialised
		({
			.format = tz::gl::image_format::BGRA32,
			.dimensions = static_cast<tz::vec2ui>(tz::window().get_dimensions()),
			.flags = {tz::gl::resource_flag::renderer_output}
		});

		tz::gl::renderer_info postprocess_info;
		postprocess_info.state().graphics.tri_count = 1;
		postprocess_info.shader().set_shader(tz::gl::shader_stage::vertex, ImportedShaderSource(blur, vertex));
		postprocess_info.shader().set_shader(tz::gl::shader_stage::fragment, ImportedShaderSource(blur, fragment));
		tz::gl::resource_handle blur_buffer_handle = postprocess_info.add_resource(blur_data);
		tz::gl::resource_handle colour_target_handle = postprocess_info.add_resource(blur_image);
		postprocess_info.set_options({tz::gl::renderer_option::no_depth_testing});
		postprocess_info.debug_name("Blur Pass");
		tz::gl::renderer_handle blur_rendererh = tz::gl::get_device().create_renderer(postprocess_info);

		tz::gl::renderer_info rinfo;
		rinfo.shader().set_shader(tz::gl::shader_stage::vertex, ImportedShaderSource(tz_triangle_demo, vertex));
		rinfo.shader().set_shader(tz::gl::shader_stage::fragment, ImportedShaderSource(tz_triangle_demo, fragment));
		rinfo.state().graphics.tri_count = 1;
		rinfo.set_options({tz::gl::renderer_option::no_depth_testing});
		rinfo.set_output(tz::gl::image_output
		{{
			.colours = {tz::gl::get_device().get_renderer(blur_rendererh).get_component(colour_target_handle)}
		}});
		rinfo.debug_name("Triangle Renderer");

		tz::gl::renderer_handle rendererh = tz::gl::get_device().create_renderer(rinfo);
		tz::gl::get_device().render_graph().timeline = {rendererh, blur_rendererh};
		tz::gl::get_device().render_graph().add_dependencies(blur_rendererh, rendererh);
		
		tz::gl::renderer& blur_renderer = tz::gl::get_device().get_renderer(blur_rendererh);
		tz::gl::renderer& renderer = tz::gl::get_device().get_renderer(rendererh);

		while(!tz::window().is_close_requested())
		{
			tz::begin_frame();
			tz::gl::get_device().render();
			BlurData& blur = blur_renderer.get_resource(blur_buffer_handle)->data_as<BlurData>().front();
			static float counter = 0.0f;
			blur.direction = tz::vec2{std::sin(counter) * 50.0f, std::cos(counter * 2.0f) * 50.0f};
			counter += 0.0003f;
			tz::end_frame();
		}
	}
	tz::terminate();
}
