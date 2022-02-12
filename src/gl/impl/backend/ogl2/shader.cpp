#if TZ_OGL
#include "gl/impl/backend/ogl2/tz_opengl.hpp"
#include "gl/impl/backend/ogl2/shader.hpp"
#include <algorithm>
#include <array>

namespace tz::gl::ogl2
{
	ShaderModule::ShaderModule(ShaderModuleInfo info):
	shader(0),
	info(info)
	{
		tz_assert(ogl2::is_initialised(), "Tried to create shader module because ogl2 was not initialised. Please submit a bug report.");
		this->shader = glCreateShader(static_cast<GLenum>(this->info.type));
		// Upload source code.
		std::array<const GLchar*, 1> source_views = {this->info.code.c_str()};
		std::array<GLint, 1> source_view_lengths = {static_cast<GLint>(this->info.code.length())};
		glShaderSource(this->shader, 1, source_views.data(), source_view_lengths.data());
	}

	ShaderModule::ShaderModule(ShaderModule&& move):
	shader(0),
	info()
	{
		*this = std::move(move);
	}

	ShaderModule::~ShaderModule()
	{
		glDeleteShader(this->shader);
	}

	ShaderModule& ShaderModule::operator=(ShaderModule&& rhs)
	{
		std::swap(this->shader, rhs.shader);
		std::swap(this->info, rhs.info);
		return *this;
	}

	ShaderModule::CompileResult ShaderModule::compile()
	{
		glCompileShader(this->shader);
		GLint success;
		glGetShaderiv(this->shader, GL_COMPILE_STATUS, &success);
		if(success == GL_TRUE)
		{
			return {.success = true, .info_log = ""};
		}
		GLint info_log_length;
		glGetShaderiv(this->shader, GL_INFO_LOG_LENGTH, &info_log_length);
		OGLString info_log(info_log_length, '\0');
		GLsizei length_used;
		glGetShaderInfoLog(this->shader, info_log.length(), &length_used, info_log.data());
		length_used++; // For null terminator.
		tz_assert(std::cmp_equal(length_used, info_log.length()), "Shader Info Log had unexpected size. GL told us %zu, actual size was %u", info_log.length(), length_used);
		return {.success = false, .info_log = info_log};
	}

	ShaderModule::NativeType ShaderModule::native() const
	{
		return this->shader;
	}

	Shader::Shader(ShaderInfo info):
	program(0),
	modules(),
	info(info)
	{
		tz_assert(ogl2::is_initialised(), "Tried to create shader program because ogl2 was not initialised. Please submit a bug report.");
		this->program = glCreateProgram();

		this->modules.reserve(info.modules.length());
		std::transform(info.modules.begin(), info.modules.end(), std::back_inserter(this->modules),
		[](const ShaderModuleInfo& info)->ShaderModule
		{
			return {info};
		});
		for(ShaderModule& shader_module : this->modules)
		{
			glAttachShader(this->program, shader_module.native());
			ShaderModule::CompileResult cpl = shader_module.compile();
			tz_assert(cpl.success, "Shader Module Compilation failed: %s", cpl.info_log.c_str());
		}
		LinkResult lnk = this->link();
		tz_assert(lnk.success, "Shader Program Linkage+Validation failed: %s", lnk.info_log.c_str());
	}

	Shader::Shader(Shader&& move):
	program(0),
	modules(),
	info()
	{
		*this = std::move(move);
	}

	Shader::~Shader()
	{
		glDeleteProgram(this->program);
	}

	Shader& Shader::operator=(Shader&& rhs)
	{
		std::swap(this->program, rhs.program);
		std::swap(this->modules, rhs.modules);
		std::swap(this->info, rhs.info);
		return *this;
	}

	Shader::LinkResult Shader::link()
	{
		glLinkProgram(this->program);
		GLint success;
		glGetProgramiv(this->program, GL_LINK_STATUS, &success);
		if(success == GL_TRUE)
		{
			return this->validate();
		}
		GLint info_log_length;
		glGetProgramiv(this->program, GL_INFO_LOG_LENGTH, &info_log_length);
		OGLString info_log(info_log_length, '\0');
		GLsizei length_used;
		glGetProgramInfoLog(this->program, info_log.length(), &length_used, info_log.data());
		length_used++; // For null terminator.
		tz_assert(std::cmp_equal(length_used, info_log.length()), "Program Info Log had unexpected size. GL told us %zu, actual size was %u", info_log.length(), length_used);
		return {.success = false, .info_log = info_log};
	}

	void Shader::use() const
	{
		glUseProgram(this->program);
	}

	Shader Shader::null()
	{
		return {nullptr};
	}

	bool Shader::is_null() const
	{
		return this->program == 0;
	}

	Shader::Shader(std::nullptr_t):
	program(0),
	modules(),
	info(){}

	Shader::LinkResult Shader::validate()
	{
		glValidateProgram(this->program);
		GLint success;
		glGetProgramiv(this->program, GL_VALIDATE_STATUS, &success);
		if(success == GL_TRUE)
		{
			return {.success = true, .info_log = ""};
		}
		GLint info_log_length;
		glGetProgramiv(this->program, GL_INFO_LOG_LENGTH, &info_log_length);
		OGLString info_log(info_log_length, '\0');
		GLsizei length_used;
		glGetProgramInfoLog(this->program, info_log.length(), &length_used, info_log.data());
		length_used++; // For null terminator.
		tz_assert(std::cmp_equal(length_used, info_log.length()), "Program Info Log had unexpected size. GL told us %zu, actual size was %u", info_log.length(), length_used);
		return {.success = false, .info_log = info_log};
	}
}

#endif // TZ_OGL
