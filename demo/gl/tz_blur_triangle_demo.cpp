#include "core/tz.hpp"
#include "core/window.hpp"
#include "core/profiling/zone.hpp"
#include "gl/device.hpp"
#include "gl/renderer.hpp"
#include "gl/resource.hpp"
#include "gl/output.hpp"
#include "gl/imported_shaders.hpp"

#include ImportedShaderHeader(blur, vertex)
#include ImportedShaderHeader(blur, fragment)
#include ImportedShaderHeader(tz_triangle_demo, vertex)
#include ImportedShaderHeader(tz_triangle_demo, fragment)

int main()
{
	tz::initialise
	({
		.name = "tz_blur_triangle_demo",
		.window =
		{
			.flags = {.resizeable = false}
		}
	});
	{
		tz::gl::Device dev;

		// Create a renderer which just draws a square with a texture applied (fragment-shader post-processing does a blur)
		struct BlurData
		{
			tz::Vec2 direction;
			float pad0[2];
		};

		tz::gl::BufferResource blur_data = tz::gl::BufferResource::from_one(BlurData{}, tz::gl::ResourceAccess::DynamicFixed);
		tz::gl::ImageResource blur_image = tz::gl::ImageResource::from_uninitialised(tz::gl::ImageFormat::BGRA32, tz::Vec2{tz::window().get_width(), tz::window().get_height()}, tz::gl::ResourceAccess::StaticFixed, {tz::gl::ResourceFlag::RendererOutput});

		tz::gl::RendererInfo postprocess_info;
		postprocess_info.shader().set_shader(tz::gl::ShaderStage::Vertex, ImportedShaderSource(blur, vertex));
		postprocess_info.shader().set_shader(tz::gl::ShaderStage::Fragment, ImportedShaderSource(blur, fragment));
		tz::gl::ResourceHandle blur_buffer_handle = postprocess_info.add_resource(blur_data);
		tz::gl::ResourceHandle colour_target_handle = postprocess_info.add_resource(blur_image);
		postprocess_info.set_options({tz::gl::RendererOption::NoDepthTesting});
		tz::gl::Renderer blur_renderer = dev.create_renderer(postprocess_info);

		tz::gl::RendererInfo rinfo;
		rinfo.shader().set_shader(tz::gl::ShaderStage::Vertex, ImportedShaderSource(tz_triangle_demo, vertex));
		rinfo.shader().set_shader(tz::gl::ShaderStage::Fragment, ImportedShaderSource(tz_triangle_demo, fragment));
		rinfo.set_options({tz::gl::RendererOption::NoDepthTesting});
		rinfo.set_output(tz::gl::ImageOutput
		{{
			.colours = {blur_renderer.get_component(colour_target_handle)}
		}});

		tz::gl::Renderer renderer = dev.create_renderer(rinfo);

		while(!tz::window().is_close_requested())
		{
			TZ_FRAME_BEGIN;
			tz::window().update();
			renderer.render(1);
			BlurData& blur = blur_renderer.get_resource(blur_buffer_handle)->data_as<BlurData>().front();
			static float counter = 0.0f;
			blur.direction = tz::Vec2{std::sin(counter) * 50.0f, std::cos(counter * 2.0f) * 50.0f};
			counter += 0.0003f;
			blur_renderer.render(1);
			TZ_FRAME_END;
		}
	}
	tz::terminate();
}
