#include "core/tz.hpp"
#include "gl/imported_shaders.hpp"
#include "gl/device.hpp"
#include "gl/renderer.hpp"
#include "gl/resource.hpp"

#include ImportedShaderHeader(empty, vertex)
#include ImportedShaderHeader(empty, fragment)

void empty_renderer(tz::gl2::Device& dev)
{
	tz::gl2::RendererInfo rinfo;
	rinfo.shader().set_shader(tz::gl2::ShaderStage::Vertex, ImportedShaderSource(empty, vertex));
	rinfo.shader().set_shader(tz::gl2::ShaderStage::Fragment, ImportedShaderSource(empty, fragment));
	tz::gl2::Renderer empty = dev.create_renderer(rinfo);
	empty.render();
}

void renderer_creation(tz::gl2::Device& dev)
{
	tz::gl2::ImageResource ires0 = tz::gl2::ImageResource::from_uninitialised(tz::gl2::ImageFormat::RGBA32, {1u, 1u});
	tz::gl2::BufferResource bres0 = tz::gl2::BufferResource::from_one(5.0f);

	tz::gl2::RendererInfo rinfo1;
	rinfo1.shader().set_shader(tz::gl2::ShaderStage::Vertex, ImportedShaderSource(empty, vertex));
	rinfo1.shader().set_shader(tz::gl2::ShaderStage::Fragment, ImportedShaderSource(empty, fragment));
	rinfo1.add_resource(ires0);
	tz::gl2::Renderer renderer1 = dev.create_renderer(rinfo1);

	tz::gl2::RendererInfo rinfo2 = rinfo1;
	rinfo2.add_resource(bres0);
	tz::gl2::Renderer renderer2 = dev.create_renderer(rinfo2);

	renderer1.render();
	renderer2.render();
}

int main()
{
	tz::initialise
	({
		.name = "tz_renderer_test",
		.app_type = tz::ApplicationType::HiddenWindowApplication
	});
	{
		tz::gl2::Device dev;
		empty_renderer(dev);
		renderer_creation(dev);
	}
	tz::terminate();
}
