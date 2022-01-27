#include "core/tz.hpp"
#include "gl/device.hpp"
#include "gl/renderer.hpp"
#include "gl/resource.hpp"
#include "gl/imported_shaders.hpp"

#include ImportedShaderFile(tz_triangle_demo.vertex)
#include ImportedShaderFile(tz_triangle_demo.fragment)

int main()
{
	tz::GameInfo g{"tz_triangle_demo (gl2)", {1, 0, 0}, tz::info()};
	tz::initialise(g, tz::ApplicationType::WindowApplication);
	{
		tz::gl2::Device dev;

		std::vector<unsigned char> imgdata = 
		{{
			0b0000'0000,
			0b0000'0000,
			0b1111'1111,
			0b1111'1111,

			0b1111'1111,
			0b0000'0000,
			0b0000'0000,
			0b1111'1111,

			0b0000'0000,
			0b1111'1111,
			0b0000'0000,
			0b1111'1111,

			0b0000'0000,
			0b0000'0000,
			0b1111'1111,
			0b1111'1111
		}};
		tz::gl2::ImageResource img = tz::gl2::ImageResource::from_memory(tz::gl2::ImageFormat::RGBA32, {2u, 2u}, std::as_bytes(static_cast<std::span<unsigned char>>(imgdata)));

		struct TriangleVertexData
		{
			tz::Vec3 position;
			float pad0;
			tz::Vec2 texcoord;
			float pad1[2];
		};

		std::array<TriangleVertexData, 3> bufdata
		{
			TriangleVertexData{.position = {-0.5f, 0.5f, 0.0f}, .texcoord = {0.0f, 0.0f}},
			TriangleVertexData{.position = {0.0f, -0.5f, 0.0f}, .texcoord = {0.5f, 1.0f}},
			TriangleVertexData{.position = {0.5f, 0.5f, 0.0f}, .texcoord = {1.0f, 0.0f}},
		};
		tz::gl2::BufferResource buf = tz::gl2::BufferResource::from_many<TriangleVertexData>(bufdata);

		tz::gl2::RendererInfo rinfo;
		rinfo.shader().set_shader(tz::gl2::ShaderStage::Vertex, GetImportedShaderSource(tz_triangle_demo_vertex));
		rinfo.shader().set_shader(tz::gl2::ShaderStage::Fragment, GetImportedShaderSource(tz_triangle_demo_fragment));
		rinfo.add_resource(img);
		rinfo.add_resource(buf);

		tz::gl2::Renderer renderer = dev.create_renderer(rinfo);

		while(!tz::window().is_close_requested())
		{
			tz::window().update();
			renderer.render(1);
		}
	}
	tz::terminate();
}
