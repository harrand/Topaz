#include "core/tz.hpp"
#include "core/window.hpp"
#include "core/profiling/zone.hpp"
#include "gl/device.hpp"
#include "gl/renderer.hpp"
#include "gl/resource.hpp"
#include "gl/imported_shaders.hpp"
#include "gl/component.hpp"

#include ImportedShaderHeader(tz_dynamic_triangle_demo, vertex)
#include ImportedShaderHeader(tz_dynamic_triangle_demo, fragment)

int main()
{
	tz::initialise
	({
		.name = "tz_triangle_demo (gl2)",
		.window = {.flags = {.resizeable = false}}
	});
	{
		tz::gl2::Device dev;
		using namespace tz::literals;
		tz::gl2::ImageResource img = tz::gl2::ImageResource::from_memory
		(
			tz::gl2::ImageFormat::RGBA32,
			{2u, 2u},
			{
				0b0000'0000_uc,
				0b0000'0000_uc,
				0b1111'1111_uc,
				0b1111'1111_uc,

				0b1111'1111_uc,
				0b0000'0000_uc,
				0b0000'0000_uc,
				0b1111'1111_uc,

				0b0000'0000_uc,
				0b1111'1111_uc,
				0b0000'0000_uc,
				0b1111'1111_uc,

				0b0000'0000_uc,
				0b0000'0000_uc,
				0b1111'1111_uc,
				0b1111'1111_uc
			}
		);

		struct TriangleVertexData
		{
			tz::Vec3 position;
			float pad0;
			tz::Vec2 texcoord;
			float pad1[2];
		};

		tz::gl2::BufferResource buf = tz::gl2::BufferResource::from_many
		({
			TriangleVertexData{.position = {-0.5f, -0.5f, 0.0f}, .texcoord = {0.0f, 0.0f}},
			TriangleVertexData{.position = {0.0f, 0.5f, 0.0f}, .texcoord = {0.5f, 1.0f}},
			TriangleVertexData{.position = {0.5f, -0.5f, 0.0f}, .texcoord = {1.0f, 0.0f}},
		}, tz::gl2::ResourceAccess::DynamicVariable);

		tz::gl2::RendererInfo rinfo;
		rinfo.shader().set_shader(tz::gl2::ShaderStage::Vertex, ImportedShaderSource(tz_dynamic_triangle_demo, vertex));
		rinfo.shader().set_shader(tz::gl2::ShaderStage::Fragment, ImportedShaderSource(tz_dynamic_triangle_demo, fragment));
		rinfo.add_resource(img);
		tz::gl2::ResourceHandle bufh = rinfo.add_resource(buf);

		tz::gl2::Renderer renderer = dev.create_renderer(rinfo);
		// TODO: Remove
		static_cast<tz::gl2::BufferComponent*>(renderer.get_component(bufh))->resize(4 * sizeof(TriangleVertexData));

		while(!tz::window().is_close_requested())
		{
			TZ_FRAME_BEGIN;
			tz::window().update();
			renderer.render(1);

			{
				// Get the top vertex of the triangle, and oscillate its height :)
				TriangleVertexData& top_vertex = renderer.get_resource(bufh)->data_as<TriangleVertexData>()[1];
				static float x = 0.0f;
				// Between -1 and -0.5
				top_vertex.position[1] = (std::sin(x += 0.05f) * 0.25f) - 0.25f;
			}
			TZ_FRAME_END;
		}
	}
	tz::terminate();
}
