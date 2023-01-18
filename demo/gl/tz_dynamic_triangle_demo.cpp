#include "tz/core/tz.hpp"
#include "hdk/profile.hpp"
#include "tz/core/time.hpp"
#include "tz/gl/device.hpp"
#include "tz/gl/renderer.hpp"
#include "tz/gl/resource.hpp"
#include "tz/gl/imported_shaders.hpp"
#include "tz/gl/component.hpp"
#include <random>
#include <algorithm>

#include ImportedShaderHeader(tz_dynamic_triangle_demo, vertex)
#include ImportedShaderHeader(tz_dynamic_triangle_demo, fragment)

struct TriangleVertexData
{
	hdk::vec3 position;
	float pad0;
	hdk::vec2 texcoord;
	float pad1[2];
};

TriangleVertexData get_random_triangle(std::default_random_engine& rand)
{
	std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
	return
	{
		.position = {dist(rand), dist(rand), dist(rand)},
		.texcoord = {std::abs(dist(rand)), std::abs(dist(rand))}
	};
}

int main()
{
	tz::initialise
	({
		.name = "tz_dynamic_triangle_demo"
	});
	{
		using namespace tz::literals;
		std::size_t triangle_count = 1;

		tz::gl::RendererInfo rinfo;
		rinfo.shader().set_shader(tz::gl::ShaderStage::Vertex, ImportedShaderSource(tz_dynamic_triangle_demo, vertex));
		rinfo.shader().set_shader(tz::gl::ShaderStage::Fragment, ImportedShaderSource(tz_dynamic_triangle_demo, fragment));
		tz::gl::ResourceHandle imgh = rinfo.add_resource
		(
			tz::gl::ImageResource::from_memory
			(
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
				{
					.format = tz::gl::image_format::RGBA32,
					.dimensions = {2u, 2u},
					.access = tz::gl::ResourceAccess::DynamicVariable,
					.flags =
					{
						tz::gl::ResourceFlag::ImageFilterLinear,
						tz::gl::ResourceFlag::ImageWrapRepeat
					}
				}
			)
		);
		tz::gl::ResourceHandle bufh = rinfo.add_resource
		(
			tz::gl::BufferResource::from_many
			(
				{
					TriangleVertexData{.position = {-0.5f, -0.5f, 0.0f}, .texcoord = {0.0f, 0.0f}},
					TriangleVertexData{.position = {0.0f, 0.5f, 0.0f}, .texcoord = {0.5f, 1.0f}},
					TriangleVertexData{.position = {0.5f, -0.5f, 0.0f}, .texcoord = {1.0f, 0.0f}},
				}, 
				{
					.access = tz::gl::ResourceAccess::DynamicVariable
				}
			)
		);
		tz::gl::ResourceHandle ibufh = rinfo.add_resource
		(
			tz::gl::BufferResource::from_many
			(
				{0u, 1u, 2u},
				{
					.access = tz::gl::ResourceAccess::DynamicVariable,
					.flags = {tz::gl::ResourceFlag::IndexBuffer}
				}
			)
		);
		rinfo.state().graphics.index_buffer = ibufh;

		tz::gl::RendererHandle rendererh = tz::gl::device().create_renderer(rinfo);
		tz::gl::Renderer& renderer = tz::gl::device().get_renderer(rendererh);
		std::default_random_engine rand;
		tz::delay fixed_update{50_ms};

		while(!tz::window().is_close_requested())
		{
			tz::begin_frame();
			renderer.render(triangle_count);
			tz::end_frame();

			static bool up = true;
			if(fixed_update.done())
			{
				HDK_PROFZONE("Dynamic Updates", 0xFF00AA00);
				// Add new triangle by resizing the triangle vertex storage buffer to a capacity large enough for an extra triangle. Then we randomise the new triangle data.
				fixed_update.reset();
				triangle_count++;
				auto img_comp = static_cast<tz::gl::ImageComponent*>(renderer.get_component(imgh));
				if(img_comp->get_dimensions()[0] > 49)
				{
					up = false;
				}
				else if(img_comp->get_dimensions()[0] < 2)
				{
					up = true;
				}
				hdk::vec2ui new_dims = img_comp->get_dimensions();
				if(up)
				{
					new_dims += {1u, 1u};
				}
				else
				{
					new_dims -= {1u, 1u};
				}

				const std::size_t vtx_count = 3 * triangle_count;
				renderer.edit(tz::gl::RendererEditBuilder{}
					.buffer_resize({.buffer_handle = bufh, .size = sizeof(TriangleVertexData) * vtx_count})
					.buffer_resize({.buffer_handle = ibufh, .size = sizeof(unsigned int) * vtx_count})
					.image_resize({.image_handle = imgh, .dimensions = new_dims})
					.build());
				// Get the resource data for the new triangle and set it to random values.
				std::span<TriangleVertexData> buf_data = renderer.get_resource(bufh)->data_as<TriangleVertexData>();
				std::span<unsigned int> idx_data = renderer.get_resource(ibufh)->data_as<unsigned int>();
				// We do this by setting the last 3 vertices to a random triangle;
				for(std::size_t i = 0; i < 3; i++)
				{
					buf_data[buf_data.size() - 3 + i] = get_random_triangle(rand);
					idx_data[idx_data.size() - 3 + i] = ((triangle_count - 1) * 3) + i;
				}

				// Shuffle each byte randomly.
				hdk::vec2ui img_dims = img_comp->get_dimensions();
				std::span<std::byte> img_data = renderer.get_resource(imgh)->data();
				std::random_shuffle(img_data.begin(), img_data.end());
				// But set the first pixel (bottom left) to always be white.
				std::fill(img_data.begin(), img_data.begin() + 4, std::byte{255});
			}
		}
	}
	tz::terminate();
}
