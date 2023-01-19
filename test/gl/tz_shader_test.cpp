#include "tz/core/tz.hpp"
#include "tz/gl/device.hpp"
#include "tz/gl/renderer.hpp"
#include "tz/gl/imported_shaders.hpp"

#include ImportedShaderHeader(shader_test, compute)

int main()
{
	tz::initialise
	({
		.name = "tz_shader_test",
		.flags = {tz::application_flag::HiddenWindow}
	});
	{
		tz::gl::renderer_info rinfo;
		rinfo.shader().set_shader(tz::gl::shader_stage::compute, ImportedShaderSource(shader_test, compute));
		tz::gl::renderer& renderer = tz::gl::get_device().get_renderer(tz::gl::get_device().create_renderer(rinfo));
		renderer.render(1);
	}
	tz::terminate();
}
