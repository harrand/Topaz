#if TZ_OGL
#include "tz/core/profile.hpp"
#include "tz/gl/impl/opengl/detail/tz_opengl.hpp"
#include "tz/gl/impl/opengl/detail/shader.hpp"
#include <algorithm>
#include <array>

namespace tz::gl::ogl2
{
	shader_module::shader_module(shader_module_info info):
	shader(0),
	info(info)
	{
		TZ_PROFZONE("ogl - shader module create", 0xFFAA0000);
		tz::assert(ogl2::is_initialised(), "Tried to create shader module because ogl2 was not initialised. Please submit a bug report.");
		this->shader = glCreateShader(static_cast<GLenum>(this->info.type));
		// Upload source code.
		std::array<const GLchar*, 1> source_views = {this->info.code.c_str()};
		std::array<GLint, 1> source_view_lengths = {static_cast<GLint>(this->info.code.length())};
		glShaderSource(this->shader, 1, source_views.data(), source_view_lengths.data());
	}

	shader_module::shader_module(shader_module&& move):
	shader(0),
	info()
	{
		*this = std::move(move);
	}

	shader_module::~shader_module()
	{
		glDeleteShader(this->shader);
	}

	shader_module& shader_module::operator=(shader_module&& rhs)
	{
		std::swap(this->shader, rhs.shader);
		std::swap(this->info, rhs.info);
		return *this;
	}

	shader_type shader_module::get_type() const
	{
		return this->info.type;
	}

	shader_module::compile_result shader_module::compile()
	{
		TZ_PROFZONE("ogl - shader module compile", 0xFFAA0000);
		glCompileShader(this->shader);
		GLint success;
		glGetShaderiv(this->shader, GL_COMPILE_STATUS, &success);
		if(success == GL_TRUE)
		{
			return {.success = true, .info_log = ""};
		}
		GLint info_log_length;
		glGetShaderiv(this->shader, GL_INFO_LOG_LENGTH, &info_log_length);
		ogl_string info_log(info_log_length, '\0');
		GLsizei length_used;
		glGetShaderInfoLog(this->shader, info_log.length(), &length_used, info_log.data());
		length_used++; // For null terminator.
		tz::assert(std::cmp_equal(length_used, info_log.length()), "Shader Info Log had unexpected size. GL told us %zu, actual size was %u", info_log.length(), length_used);
		return {.success = false, .info_log = info_log};
	}

	shader_module::NativeType shader_module::native() const
	{
		return this->shader;
	}

	shader::shader(shader_info info):
	program(0),
	modules(),
	info(info)
	{
		TZ_PROFZONE("ogl - shader create", 0xFFAA0000);
		tz::assert(ogl2::is_initialised(), "Tried to create shader program because ogl2 was not initialised. Please submit a bug report.");
		this->program = glCreateProgram();

		this->modules.reserve(info.modules.length());
		std::transform(info.modules.begin(), info.modules.end(), std::back_inserter(this->modules),
		[](const shader_module_info& minfo)->shader_module
		{
			return {minfo};
		});
		for(shader_module& shader_module : this->modules)
		{
			glAttachShader(this->program, shader_module.native());
			shader_module::compile_result cpl = shader_module.compile();
			tz::assert(cpl.success, "Shader Module Compilation failed: %s", cpl.info_log.c_str());
		}
		link_result lnk = this->link();
		tz::assert(lnk.success, "Shader Program Linkage+Validation failed: %s", lnk.info_log.c_str());
	}

	shader::shader(shader&& move):
	program(0),
	modules(),
	info()
	{
		*this = std::move(move);
	}

	shader::~shader()
	{
		glDeleteProgram(this->program);
	}

	shader& shader::operator=(shader&& rhs)
	{
		std::swap(this->program, rhs.program);
		std::swap(this->modules, rhs.modules);
		std::swap(this->info, rhs.info);
		return *this;
	}

	shader::link_result shader::link()
	{
		TZ_PROFZONE("ogl - shader link", 0xFFAA0000);
		glLinkProgram(this->program);
		GLint success;
		glGetProgramiv(this->program, GL_LINK_STATUS, &success);
		if(success == GL_TRUE)
		{
			return this->validate();
		}
		GLint info_log_length;
		glGetProgramiv(this->program, GL_INFO_LOG_LENGTH, &info_log_length);
		ogl_string info_log(info_log_length, '\0');
		GLsizei length_used;
		glGetProgramInfoLog(this->program, info_log.length(), &length_used, info_log.data());
		length_used++; // For null terminator.
		tz::assert(std::cmp_equal(length_used, info_log.length()), "Program Info Log had unexpected size. GL told us %zu, actual size was %u", info_log.length(), length_used);
		return {.success = false, .info_log = info_log};
	}

	void shader::use() const
	{
		TZ_PROFZONE("ogl - shader use", 0xFFAA0000);
		glUseProgram(this->program);
	}

	shader shader::null()
	{
		return {nullptr};
	}

	bool shader::is_null() const
	{
		return this->program == 0;
	}

	bool shader::is_compute() const
	{
		return std::any_of(this->info.modules.begin(), this->info.modules.end(),
		[](const shader_module_info& mod)
		{
			return mod.type == shader_type::compute;
		});
	}

	bool shader::has_tessellation() const
	{
		return std::any_of(this->info.modules.begin(), this->info.modules.end(),
		[](const shader_module_info& mod)
		{
			return mod.type == shader_type::tessellation_control || mod.type == shader_type::tessellation_evaluation;
		});
	}

	std::string shader::debug_get_name() const
	{
		return this->debug_name;
	}

	void shader::debug_set_name(std::string name)
	{
		this->debug_name = name;
		#if TZ_DEBUG
			glObjectLabel(GL_PROGRAM, this->program, -1, this->debug_name.c_str());
			for(const shader_module& s_module : this->modules)
			{
				const char* extension;
				switch(s_module.get_type())
				{
					case shader_type::vertex:
						extension = "vertex";
					break;
					case shader_type::tessellation_control:
						extension = "tesscon";
					break;
					case shader_type::tessellation_evaluation:
						extension = "tesseval";
					break;
					case shader_type::fragment:
						extension = "fragment";
					break;
					case shader_type::compute:
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

	shader::shader(std::nullptr_t):
	program(0),
	modules(),
	info(){}

	shader::link_result shader::validate()
	{
		TZ_PROFZONE("ogl - shader validate", 0xFFAA0000);
		glValidateProgram(this->program);
		GLint success;
		glGetProgramiv(this->program, GL_VALIDATE_STATUS, &success);
		if(success == GL_TRUE)
		{
			return {.success = true, .info_log = ""};
		}
		GLint info_log_length;
		glGetProgramiv(this->program, GL_INFO_LOG_LENGTH, &info_log_length);
		ogl_string info_log(info_log_length, '\0');
		GLsizei length_used;
		glGetProgramInfoLog(this->program, info_log.length(), &length_used, info_log.data());
		length_used++; // For null terminator.
		tz::assert(std::cmp_equal(length_used, info_log.length()), "Program Info Log had unexpected size. GL told us %zu, actual size was %u", info_log.length(), length_used);
		return {.success = false, .info_log = info_log};
	}
}

#endif // TZ_OGL
