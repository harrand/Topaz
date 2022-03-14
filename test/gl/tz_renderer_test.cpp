#include "core/tz.hpp"
#include "gl/imported_shaders.hpp"
#include "gl/device.hpp"
#include "gl/renderer.hpp"
#include "gl/resource.hpp"

#include ImportedShaderHeader(empty, vertex)
#include ImportedShaderHeader(empty, fragment)

void empty_renderer(tz::gl::Device& dev)
{
	tz::gl::RendererInfo rinfo;
	rinfo.shader().set_shader(tz::gl::ShaderStage::Vertex, ImportedShaderSource(empty, vertex));
	rinfo.shader().set_shader(tz::gl::ShaderStage::Fragment, ImportedShaderSource(empty, fragment));
	tz::gl::Renderer empty = dev.create_renderer(rinfo);
	empty.render();
}

void renderer_creation(tz::gl::Device& dev)
{
	tz::gl::BufferResource bres0 = tz::gl::BufferResource::from_one(5.0f);

	tz::gl::RendererInfo rinfo1;
	rinfo1.shader().set_shader(tz::gl::ShaderStage::Vertex, ImportedShaderSource(empty, vertex));
	rinfo1.shader().set_shader(tz::gl::ShaderStage::Fragment, ImportedShaderSource(empty, fragment));
	tz::gl::Renderer renderer1 = dev.create_renderer(rinfo1);

	tz::gl::RendererInfo rinfo2 = rinfo1;
	rinfo2.add_resource(bres0);
	tz::gl::Renderer renderer2 = dev.create_renderer(rinfo2);

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
		tz::gl::Device dev;
		empty_renderer(dev);
		renderer_creation(dev);
	}
	tz::terminate();
}
