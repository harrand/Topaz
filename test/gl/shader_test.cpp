#include "core/tz.hpp"
#include "core/assert.hpp"
#include "gl/device.hpp"
#include "gl/shader.hpp"

int main()
{
	tz::initialise({"tz_shader_test", tz::Version{1, 0, 0}, tz::info()}, tz::ApplicationType::Headless);
	{
		// Simply create some basic shaders. Don't try and render or anything.
		tz::gl::Device device{tz::gl::DeviceBuilder{}};

		tz::gl::ShaderBuilder builder;
		builder.set_shader_file(tz::gl::ShaderType::VertexShader, ".\\test\\gl\\shader_test.vertex.tzsl");
		builder.set_shader_file(tz::gl::ShaderType::FragmentShader, ".\\test\\gl\\shader_test.fragment.tzsl");
		tz::gl::Shader shader = device.create_shader(builder);

		const tz::gl::ShaderMeta& meta = shader.get_meta();
		tz_assert(meta.try_get_meta_value(0).value_or(tz::gl::ShaderMetaValue::Count) == tz::gl::ShaderMetaValue::UBO, "Shader failed to generate correct meta");
		tz_assert(meta.try_get_meta_value(1).value_or(tz::gl::ShaderMetaValue::Count) == tz::gl::ShaderMetaValue::SSBO, "Shader failed to generate correct meta");
	}
	tz::terminate();
}