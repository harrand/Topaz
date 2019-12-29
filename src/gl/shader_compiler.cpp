#include "gl/shader_compiler.hpp"
#include "gl/shader.hpp"
#include "core/debug/assert.hpp"

namespace tz::gl
{
	ShaderCompilerDiagnostic::ShaderCompilerDiagnostic(bool success, std::string info_log): success(success), info_log(info_log){}

	bool ShaderCompilerDiagnostic::successful() const
	{
		return this->success;
	}

	const std::string& ShaderCompilerDiagnostic::get_info_log() const
	{
		return this->info_log;
	}

	ShaderCompiler::ShaderCompiler(ShaderCompilerOptions options): options(options){}

	ShaderCompilerDiagnostic ShaderCompiler::compile(const Shader& shader) const
	{
		topaz_assert(shader.has_source(), "tz::gl::ShaderCompile::compile(...): Cannot compile Shader with empty source!");
		glCompileShader(shader.handle);

		int success_value;
		glGetShaderiv(shader.handle, GL_COMPILE_STATUS, &success_value);
		bool success = success_value == GL_TRUE ? true : false;
		std::string info_log;
		info_log.resize(1024); // Hardcode 1 KiB -- Should be absolutely plenty.
		GLsizei log_length;
		glGetShaderInfoLog(shader.handle, info_log.size(), &log_length, info_log.data());
		info_log.resize(static_cast<std::size_t>(log_length)); // Resize to the actual length of the message.

		return {success, std::move(info_log)};

	}
}