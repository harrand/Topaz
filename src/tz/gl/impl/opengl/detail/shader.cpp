#if TZ_OGL
#include "hdk/profile.hpp"
#include "tz/gl/impl/opengl/detail/tz_opengl.hpp"
#include "tz/gl/impl/opengl/detail/shader.hpp"
#include <algorithm>
#include <array>

namespace tz::gl::ogl2
{
	ShaderModule::ShaderModule(ShaderModuleInfo info):
	shader(0),
	info(info)
	{
		HDK_PROFZONE("OpenGL Backend - ShaderModule Create", 0xFFAA0000);
		hdk::assert(ogl2::is_initialised(), "Tried to create shader module because ogl2 was not initialised. Please submit a bug report.");
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

	ShaderType ShaderModule::get_type() const
	{
		return this->info.type;
	}

	ShaderModule::CompileResult ShaderModule::compile()
	{
		HDK_PROFZONE("OpenGL Backend - Shader Source Compile", 0xFFAA0000);
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
		hdk::assert(std::cmp_equal(length_used, info_log.length()), "Shader Info Log had unexpected size. GL told us %zu, actual size was %u", info_log.length(), length_used);
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
		HDK_PROFZONE("OpenGL Backend - Shader Create", 0xFFAA0000);
		hdk::assert(ogl2::is_initialised(), "Tried to create shader program because ogl2 was not initialised. Please submit a bug report.");
		this->program = glCreateProgram();

		this->modules.reserve(info.modules.length());
		std::transform(info.modules.begin(), info.modules.end(), std::back_inserter(this->modules),
		[](const ShaderModuleInfo& minfo)->ShaderModule
		{
			return {minfo};
		});
		for(ShaderModule& shader_module : this->modules)
		{
			glAttachShader(this->program, shader_module.native());
			ShaderModule::CompileResult cpl = shader_module.compile();
			hdk::assert(cpl.success, "Shader Module Compilation failed: %s", cpl.info_log.c_str());
		}
		LinkResult lnk = this->link();
		hdk::assert(lnk.success, "Shader Program Linkage+Validation failed: %s", lnk.info_log.c_str());
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
		HDK_PROFZONE("OpenGL Backend - Shader Link", 0xFFAA0000);
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
		hdk::assert(std::cmp_equal(length_used, info_log.length()), "Program Info Log had unexpected size. GL told us %zu, actual size was %u", info_log.length(), length_used);
		return {.success = false, .info_log = info_log};
	}

	void Shader::use() const
	{
		HDK_PROFZONE("OpenGL Backend - Shader Use", 0xFFAA0000);
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

	bool Shader::is_compute() const
	{
		return std::any_of(this->info.modules.begin(), this->info.modules.end(),
		[](const ShaderModuleInfo& mod)
		{
			return mod.type == ShaderType::Compute;
		});
	}

	bool Shader::has_tessellation() const
	{
		return std::any_of(this->info.modules.begin(), this->info.modules.end(),
		[](const ShaderModuleInfo& mod)
		{
			return mod.type == ShaderType::TessellationControl || mod.type == ShaderType::TessellationEvaluation;
		});
	}

	std::string Shader::debug_get_name() const
	{
		return this->debug_name;
	}

	void Shader::debug_set_name(std::string name)
	{
		this->debug_name = name;
		#if HDK_DEBUG
			glObjectLabel(GL_PROGRAM, this->program, -1, this->debug_name.c_str());
			for(const ShaderModule& s_module : this->modules)
			{
				const char* extension;
				switch(s_module.get_type())
				{
					case ShaderType::Vertex:
						extension = "vertex";
					break;
					case ShaderType::TessellationControl:
						extension = "tesscon";
					break;
					case ShaderType::TessellationEvaluation:
						extension = "tesseval";
					break;
					case ShaderType::Fragment:
						extension = "fragment";
					break;
					case ShaderType::Compute:
						extension = "compute";
					break;
					default:
						extension = "unknown-shader-type";
					break;
				}
				std::string mod_name = this->debug_name + "." + extension;
				glObjectLabel(GL_SHADER, s_module.native(), -1, mod_name.c_str());
			}
		#endif
	}

	Shader::Shader(std::nullptr_t):
	program(0),
	modules(),
	info(){}

	Shader::LinkResult Shader::validate()
	{
		HDK_PROFZONE("OpenGL Backend - Shader Validate", 0xFFAA0000);
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
		hdk::assert(std::cmp_equal(length_used, info_log.length()), "Program Info Log had unexpected size. GL told us %zu, actual size was %u", info_log.length(), length_used);
		return {.success = false, .info_log = info_log};
	}
}

#endif // TZ_OGL
