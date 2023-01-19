#include "tz/core/tz.hpp"
#include "tz/gl/renderer.hpp"
#include "tz/gl/device.hpp"
#include "tz/gl/resource.hpp"
#include "tz/gl/imported_shaders.hpp"
#include "tz/gl/output.hpp"
#include "tz/dbgui/dbgui.hpp"

#include ImportedShaderHeader(tz_bloom_demo_combine, vertex)
#include ImportedShaderHeader(tz_bloom_demo_combine, fragment)
#include ImportedShaderHeader(tz_bloom_demo, vertex)
#include ImportedShaderHeader(tz_bloom_demo, fragment)

struct RenderData
{
	hdk::vec2 triangle_pos = {-0.5f, 0.0f};
	hdk::vec2 triangle_scale = {1.0f, 1.0f};
	hdk::vec2 quad_pos = {0.5f, 0.0f};
	hdk::vec2 quad_scale = {1.0f, 1.0f};

	hdk::vec3 triangle_colour = {1.0f, 0.0f, 0.0f};
	float pad0;
	hdk::vec3 quad_colour = {0.0f, 0.0f, 1.0f};
	float pad1;
};

void dbgui(RenderData& data)
{
	if(ImGui::CollapsingHeader("Triangle"))
	{
		ImGui::DragFloat2("Position", data.triangle_pos.data().data(), 0.01f, -1.0f, 1.0f);
		ImGui::DragFloat2("Scale", data.triangle_scale.data().data(), 0.005f, 0.0f, 1.0f);
		ImGui::ColorEdit3("Colour", data.triangle_colour.data().data());
	}
	if(ImGui::CollapsingHeader("Square"))
	{
		ImGui::DragFloat2("Position", data.quad_pos.data().data(), 0.01f, -1.0f, 1.0f);
		ImGui::DragFloat2("Scale", data.quad_scale.data().data(), 0.005f, 0.0f, 1.0f);
		ImGui::ColorEdit3("Colour", data.quad_colour.data().data());
	}
}

int main()
{

	tz::initialise
	({
		.name = "tz_bloom_demo",
		.flags = {tz::application_flag::UnresizeableWindow}
	});
	{
		struct BloomOptions
		{
			float threshold = 0.7f;
			float blur_length = 8.0f;
			std::uint32_t iterations = 6u;
			float pad0;
		};
		// This demo uses the following
		// Renderer to combine two images into one and render it to the screen.
		tz::gl::ImageResource image_out0 = tz::gl::ImageResource::from_uninitialised
		({
			.format = tz::gl::image_format::BGRA32,
			.dimensions = static_cast<hdk::vec2ui>(tz::window().get_dimensions()),
			.flags = {tz::gl::resource_flag::renderer_output}
		});
		tz::gl::ImageResource image_out1 = image_out0;

		tz::gl::BufferResource bloom_data_buffer = tz::gl::BufferResource::from_one(BloomOptions{},
		{
			.access = tz::gl::resource_access::dynamic_fixed
		});

		tz::gl::renderer_info combine_info;
		tz::gl::resource_handle bloom_data_handle = combine_info.add_resource(bloom_data_buffer);
		tz::gl::resource_handle iout0h = combine_info.add_resource(image_out0);
		tz::gl::resource_handle iout1h = combine_info.add_resource(image_out1);
		combine_info.shader().set_shader(tz::gl::shader_stage::vertex, ImportedShaderSource(tz_bloom_demo_combine, vertex));
		combine_info.shader().set_shader(tz::gl::shader_stage::fragment, ImportedShaderSource(tz_bloom_demo_combine, fragment));
		tz::gl::renderer_handle combineh = tz::gl::get_device().create_renderer(combine_info);

		// Firstly draw some shapes. Brighter pixels are written into a second colour attachment
		tz::gl::BufferResource render_data = tz::gl::BufferResource::from_one(RenderData{},
		{
			.access = tz::gl::resource_access::dynamic_fixed
		});

		tz::gl::renderer_info rinfo;
		rinfo.shader().set_shader(tz::gl::shader_stage::vertex, ImportedShaderSource(tz_bloom_demo, vertex));
		rinfo.shader().set_shader(tz::gl::shader_stage::fragment, ImportedShaderSource(tz_bloom_demo, fragment));
		tz::gl::resource_handle render_bufh = rinfo.add_resource(render_data);

		tz::gl::Renderer& combine_old = tz::gl::get_device().get_renderer(combineh);

		tz::gl::resource_handle bloom_bufh = rinfo.ref_resource(combine_old.get_component(bloom_data_handle));
		rinfo.set_output(tz::gl::ImageOutput
		{{
			.colours = {combine_old.get_component(iout0h), combine_old.get_component(iout1h)}
		}});
		tz::gl::renderer_handle rendererh = tz::gl::get_device().create_renderer(rinfo);

		// Blur the second colour attachment
		// TODO
		
		// Do a final pass, essentially adding both images together. Present that image
		// TODO

		// Debug UI
		bool menu_enabled = false;
		bool bloom_menu_enabled = false;
		tz::dbgui::game_menu().add_callback([&menu_enabled, &bloom_menu_enabled]()
		{
			ImGui::MenuItem("Shapes", nullptr, &menu_enabled);
			ImGui::MenuItem("Bloom", nullptr, &bloom_menu_enabled);
		});

		tz::gl::Renderer& combine = tz::gl::get_device().get_renderer(combineh);
		tz::gl::Renderer& renderer = tz::gl::get_device().get_renderer(rendererh);

		while(!tz::window().is_close_requested())
		{
			tz::begin_frame();
			renderer.render(3);
			combine.render(1);

			tz::dbgui::run([&menu_enabled, &bloom_menu_enabled, &bloom_data_handle, &combine, &render_bufh, &renderer]()
			{
				if(menu_enabled)
				{
					ImGui::Begin("Shapes", &menu_enabled);
					dbgui(renderer.get_resource(render_bufh)->data_as<RenderData>().front());
					ImGui::End();
				}
				if(bloom_menu_enabled)
				{
					BloomOptions& bloom = combine.get_resource(bloom_data_handle)->data_as<BloomOptions>().front();
					int iters = bloom.iterations;
					ImGui::Begin("Bloom", &bloom_menu_enabled);
					ImGui::DragFloat("Threshold", &bloom.threshold, 0.005f, 0.0f, 1.0f);
					ImGui::DragFloat("Blur Length", &bloom.blur_length, 0.05f, 0.0f, 50.0f);
					if(ImGui::DragInt("Iteration Count", &iters, 1, 0, 128))
					{
						bloom.iterations = iters;
					}
					ImGui::End();
				}
			});
			tz::end_frame();
		}
	}
	tz::terminate();
}
