#include "tz/core/tz.hpp"
#include "hdk/profile.hpp"
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
		.flags = {tz::application_flag::UnresizeableWindow}
	});
	{
		// Create a renderer which just draws a square with a texture applied (fragment-shader post-processing does a blur)
		struct BlurData
		{
			hdk::vec2 direction;
			float pad0[2];
		};

		tz::gl::BufferResource blur_data = tz::gl::BufferResource::from_one(BlurData{},
		{
			.access = tz::gl::resource_access::dynamic_fixed
		});
		tz::gl::ImageResource blur_image = tz::gl::ImageResource::from_uninitialised
		({
			.format = tz::gl::image_format::BGRA32,
			.dimensions = static_cast<hdk::vec2ui>(tz::window().get_dimensions()),
			.flags = {tz::gl::resource_flag::renderer_output}
		});

		tz::gl::renderer_info postprocess_info;
		postprocess_info.shader().set_shader(tz::gl::ShaderStage::Vertex, ImportedShaderSource(blur, vertex));
		postprocess_info.shader().set_shader(tz::gl::ShaderStage::Fragment, ImportedShaderSource(blur, fragment));
		tz::gl::resource_handle blur_buffer_handle = postprocess_info.add_resource(blur_data);
		tz::gl::resource_handle colour_target_handle = postprocess_info.add_resource(blur_image);
		postprocess_info.set_options({tz::gl::renderer_option::NoDepthTesting});
		tz::gl::renderer_handle blur_rendererh = tz::gl::device().create_renderer(postprocess_info);

		tz::gl::renderer_info rinfo;
		rinfo.shader().set_shader(tz::gl::ShaderStage::Vertex, ImportedShaderSource(tz_triangle_demo, vertex));
		rinfo.shader().set_shader(tz::gl::ShaderStage::Fragment, ImportedShaderSource(tz_triangle_demo, fragment));
		rinfo.set_options({tz::gl::renderer_option::NoDepthTesting});
		rinfo.set_output(tz::gl::ImageOutput
		{{
			.colours = {tz::gl::device().get_renderer(blur_rendererh).get_component(colour_target_handle)}
		}});

		tz::gl::renderer_handle rendererh = tz::gl::device().create_renderer(rinfo);
		
		tz::gl::Renderer& blur_renderer = tz::gl::device().get_renderer(blur_rendererh);
		tz::gl::Renderer& renderer = tz::gl::device().get_renderer(rendererh);

		while(!tz::window().is_close_requested())
		{
			tz::begin_frame();
			renderer.render(1);
			BlurData& blur = blur_renderer.get_resource(blur_buffer_handle)->data_as<BlurData>().front();
			static float counter = 0.0f;
			blur.direction = hdk::vec2{std::sin(counter) * 50.0f, std::cos(counter * 2.0f) * 50.0f};
			counter += 0.0003f;
			blur_renderer.render(1);
			tz::end_frame();
		}
	}
	tz::terminate();
}
