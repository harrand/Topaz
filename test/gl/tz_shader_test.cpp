#include "core/tz.hpp"
#include "gl/device.hpp"
#include "gl/renderer.hpp"
#include "gl/imported_shaders.hpp"

#include ImportedShaderHeader(shader_test, compute)

int main()
{
	tz::initialise
	({
		.name = "tz_shader_test",
		.flags = {tz::ApplicationFlag::HiddenWindow}
	});
	{
		tz::gl::Device dev;
		tz::gl::RendererInfo rinfo;
		rinfo.shader().set_shader(tz::gl::ShaderStage::Compute, ImportedShaderSource(shader_test, compute));
		tz::gl::Renderer renderer = dev.create_renderer(rinfo);
		renderer.render(1);
	}
	tz::terminate();
}
