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

	ShaderCompilerDiagnostic ShaderCompiler::compile(Shader& shader) const
	{
		topaz_assert(shader.has_source(), "tz::gl::ShaderCompiler::compile(...): Cannot compile Shader with empty source!");
		glCompileShader(shader.handle);

		int success_value;
		glGetShaderiv(shader.handle, GL_COMPILE_STATUS, &success_value);
		bool success = success_value == GL_TRUE ? true : false;
		std::string info_log;
		info_log.resize(1024); // Hardcode 1 KiB -- Should be absolutely plenty.
		GLsizei log_length;
		glGetShaderInfoLog(shader.handle, info_log.size(), &log_length, info_log.data());
		info_log.resize(static_cast<std::size_t>(log_length)); // Resize to the actual length of the message.

		shader.compilation_successful = success;
		return {success, std::move(info_log)};
	}

	ShaderCompilerDiagnostic ShaderCompiler::link(ShaderProgram& program) const
	{
		topaz_assert(program.linkable(), "tz::gl::ShaderCompiler::link(...): Cannot link program whose Shaders aren't all successfully compiled!");
		
		auto han = program.handle;

		auto verify_boilerplate = [han](GLenum pname)->ShaderCompilerDiagnostic
		{
			int success_value;
			glGetProgramiv(han, pname, &success_value);
			bool success = success_value == GL_TRUE ? true : false;
			std::string info_log;
			info_log.resize(1024); // Hardcode 1 KiB -- Should be absolutely plenty.
			GLsizei log_length;
			glGetProgramInfoLog(han, info_log.size(), &log_length, info_log.data());
			info_log.resize(static_cast<std::size_t>(log_length)); // Resize to the actual length of the message.

			return {success, std::move(info_log)};
		};

		glLinkProgram(program.handle);

		{
			auto diagnostic = verify_boilerplate(GL_LINK_STATUS);
			if(!diagnostic.successful())
				return diagnostic;
		}

		// Otherwise, continue to validation.
		glValidateProgram(program.handle);
		{
			auto diagnostic = verify_boilerplate(GL_VALIDATE_STATUS);
			if(diagnostic.successful())
				program.ready = true; // It's good to go!
			return diagnostic;
		}	
	}
}