#include "core/tz.hpp"
#include "gl/imported_shaders.hpp"
#include "gl/device.hpp"
#include "gl/renderer.hpp"

#include ImportedShaderHeader(empty, vertex)
#include ImportedShaderHeader(empty, fragment)

void empty_renderer(tz::gl2::Device& dev)
{
	tz::gl2::RendererInfo rinfo;
	rinfo.shader().set_shader(tz::gl2::ShaderStage::Vertex, ImportedShaderSource(empty, vertex));
	rinfo.shader().set_shader(tz::gl2::ShaderStage::Fragment, ImportedShaderSource(empty, fragment));
	tz::gl2::Renderer empty = dev.create_renderer(rinfo);

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
	}
	tz::terminate();
}
