#include "core/tz.hpp"
#include "core/window.hpp"
#include "core/profiling/zone.hpp"
#include "core/containers/grid_view.hpp"
#include "gl/device.hpp"
#include "gl/renderer.hpp"
#include "gl/resource.hpp"
#include "gl/imported_shaders.hpp"
#include "gl/component.hpp"
#include "gl/output.hpp"
#include <random>

#include ImportedShaderHeader(fst, vertex)
#include ImportedShaderHeader(fst, fragment)
#include ImportedShaderHeader(tz_dynamic_triangle_demo, vertex)
#include ImportedShaderHeader(tz_dynamic_triangle_demo, fragment)

struct TriangleVertexData
{
	tz::Vec3 position;
	float pad0;
	tz::Vec2 texcoord;
	float pad1[2];
};

TriangleVertexData get_random_triangle(std::default_random_engine& rand)
{
	std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
	return
	{
		.position = {dist(rand), dist(rand), dist(rand)},
		.texcoord = {dist(rand), dist(rand)}
	};
}

int main()
{
	tz::initialise
	({
		.name = "tz_dynamic_triangle_demo (gl)"
	});
	{
		tz::gl::Device dev;
		using namespace tz::literals;

		// Create a renderer which draws a texture straight to the screen.
		tz::gl::RendererInfo rtt_info;
		auto ires = tz::gl::ImageResource::from_uninitialised(tz::gl::ImageFormat::BGRA32, tz::Vec2{tz::window().get_width(), tz::window().get_height()}, tz::gl::ResourceAccess::StaticFixed, {tz::gl::ResourceFlag::RendererOutput});
		tz::gl::ResourceHandle render_to_texture_handle = rtt_info.add_resource(ires);
		rtt_info.shader().set_shader(tz::gl::ShaderStage::Vertex, ImportedShaderSource(fst, vertex));
		rtt_info.shader().set_shader(tz::gl::ShaderStage::Fragment, ImportedShaderSource(fst, fragment));
		tz::gl::Renderer rtt = dev.create_renderer(rtt_info);
		tz::gl::ImageOutput render_to_texture
		{{
			.colours = {rtt.get_component(render_to_texture_handle)},
			.depth = nullptr
		}};

		tz::gl::ImageResource img = tz::gl::ImageResource::from_memory
		(
			tz::gl::ImageFormat::RGBA32,
			{2u, 2u},
			{
				0b1111'1111_uc,
				0b0000'0000_uc,
				0b0000'0000_uc,
				0b1111'1111_uc,

				0b1111'1111_uc,
				0b1111'1111_uc,
				0b0000'0000_uc,
				0b1111'1111_uc,

				0b0000'0000_uc,
				0b0000'0000_uc,
				0b1111'1111_uc,
				0b1111'1111_uc,

				0b0000'0000_uc,
				0b1111'1111_uc,
				0b0000'0000_uc,
				0b1111'1111_uc
			},
			tz::gl::ResourceAccess::DynamicFixed
		);

		std::size_t triangle_count = 1;
		tz::gl::BufferResource buf = tz::gl::BufferResource::from_many
		({
			TriangleVertexData{.position = {-0.5f, -0.5f, -1.0f}, .texcoord = {0.0f, 0.0f}},
			TriangleVertexData{.position = {0.0f, 0.5f, -1.0f}, .texcoord = {0.5f, 1.0f}},
			TriangleVertexData{.position = {0.5f, -0.5f, -1.0f}, .texcoord = {1.0f, 0.0f}},
		}, tz::gl::ResourceAccess::DynamicVariable);
		tz::gl::BufferResource ibuf = tz::gl::BufferResource::from_many
		({
			0u, 1u, 2u
		}, tz::gl::ResourceAccess::DynamicVariable, {tz::gl::ResourceFlag::IndexBuffer});

		tz::gl::RendererInfo rinfo;
		rinfo.shader().set_shader(tz::gl::ShaderStage::Vertex, ImportedShaderSource(tz_dynamic_triangle_demo, vertex));
		rinfo.shader().set_shader(tz::gl::ShaderStage::Fragment, ImportedShaderSource(tz_dynamic_triangle_demo, fragment));
		tz::gl::ResourceHandle imgh = rinfo.add_resource(img);
		tz::gl::ResourceHandle bufh = rinfo.add_resource(buf);
		tz::gl::ResourceHandle ibufh = rinfo.add_resource(ibuf);
		rinfo.set_output(render_to_texture);

		tz::gl::Renderer renderer = dev.create_renderer(rinfo);
		std::default_random_engine rand;

		while(!tz::window().is_close_requested())
		{
			TZ_FRAME_BEGIN;
			tz::window().update();
			renderer.render(triangle_count);
			rtt.render(2);
			// Every 10k frames, add a new triangle at a random position.
			static int counter = 0;
			if(counter++ > 500)
			{
				TZ_PROFZONE("Dynamic Updates", TZ_PROFCOL_GREEN);
				// Add new triangle by resizing the triangle vertex storage buffer to a capacity large enough for an extra triangle. Then we randomise the new triangle data.
				counter = 0;
				triangle_count++;
				tz::gl::RendererEditRequest renderer_edit
				{
					.component_edits =
					{
						tz::gl::RendererBufferComponentEditRequest
						{
							.buffer_handle = bufh,
							.size = sizeof(TriangleVertexData) * 3 * triangle_count
						},
						tz::gl::RendererBufferComponentEditRequest
						{
							.buffer_handle = ibufh,
							.size = sizeof(unsigned int) * 3 * triangle_count
						}
					}
				};
				renderer.edit(renderer_edit);
				// Get the resource data for the new triangle and set it to random values.
				std::span<TriangleVertexData> buf_data = renderer.get_resource(bufh)->data_as<TriangleVertexData>();
				std::span<unsigned int> idx_data = renderer.get_resource(ibufh)->data_as<unsigned int>();
				// We do this by setting the last 3 vertices to a random triangle;
				for(std::size_t i = 0; i < 3; i++)
				{
					buf_data[buf_data.size() - 3 + i] = get_random_triangle(rand);
					idx_data[idx_data.size() - 3 + i] = ((triangle_count - 1) * 3) + i;
				}

				// While we're at it, randomise the second pixel of the image.
				std::span<std::byte> img_data = renderer.get_resource(imgh)->data();
				tz::GridView<std::byte, 4> img_view{img_data, img.get_dimensions()};
				std::span<std::byte> pixel_data = img_view(1, 0);
				std::random_shuffle(pixel_data.begin(), pixel_data.end());
			}

			TZ_FRAME_END;
		}
	}
	tz::terminate();
}
