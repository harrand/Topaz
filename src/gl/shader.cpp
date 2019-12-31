#include "gl/shader.hpp"
#include "core/debug/assert.hpp"
#include <algorithm>

namespace tz::gl
{
	ShaderProgram::ShaderProgram(): handle(glCreateProgram()), shaders(), ready(false)
	{
		this->nullify_all();
	}

	ShaderProgram::ShaderProgram(ShaderProgram&& move): handle(move.handle), shaders(std::move(move.shaders))
	{
		move.nullify_all();
		move.handle = 0;
	}

	ShaderProgram::~ShaderProgram()
	{
		// Detach all child handles. Only do this if we have a valid handle!
		if(this->handle != 0)
		{
			for(const auto& child_ptr : this->shaders)
			{
				if(child_ptr == nullptr)
					continue;
				glDetachShader(this->handle, child_ptr->handle);
			}
		}
		// Will silently ignore zero'd handles.
		glDeleteProgram(this->handle);
	}

	ShaderProgram& ShaderProgram::operator=(ShaderProgram&& rhs)
	{
		std::swap(this->handle, rhs.handle);
		std::swap(this->shaders, rhs.shaders);
		return *this;
	}

	void ShaderProgram::set(ShaderType type, std::unique_ptr<Shader> shader)
	{
		topaz_assert(type != ShaderType::NUM_TYPES, "tz::gl::ShaderProgram::set(...): Was given type index ", static_cast<std::size_t>(type), " (ShaderTypes::NUM_TYPES) which is not a legal parameter here.");
		this->shaders[static_cast<std::size_t>(type)] = std::move(shader);
	}

	void ShaderProgram::define(std::size_t index, const GLchar* name)
	{
		this->verify();
		glBindAttribLocation(this->handle, index, name);
	}

	bool ShaderProgram::linkable() const
	{
		// Usable shaders have a vertex and fragment component at the very least.
		bool enough_shaders = this->has_shader(ShaderType::Vertex)
						   && this->has_shader(ShaderType::Fragment);
		if(!enough_shaders)
			return false;
		// All attached shaders must have been compiled successfully.
		for(const auto& shader_ptr : this->shaders)
		{
			if(shader_ptr != nullptr)
			{
				if(!shader_ptr->compiled())
					return false;
			}
		}
		return true;
	}

	bool ShaderProgram::usable() const
	{
		return this->ready;
	}

	void ShaderProgram::bind()
	{
		topaz_assert(this->usable(), "tz::gl::ShaderProgram::bind(): Attempted to bind but the program is not currently usable. Make sure the program is *correctly* linked & validated before invoking this.");
		glUseProgram(this->handle);
	}

	bool ShaderProgram::has_shader(ShaderType type) const
	{
		topaz_assert(type != ShaderType::NUM_TYPES, "tz::gl::ShaderProgram::has_shader(...): Was given type index ", static_cast<std::size_t>(type), " (ShaderTypes::NUM_TYPES) which is not a legal parameter here.");
		return this->shaders[static_cast<std::size_t>(type)] != nullptr;
	}

	bool ShaderProgram::operator==(ShaderProgramHandle rhs) const
	{
		return this->handle == rhs;
	}

	bool ShaderProgram::operator!=(ShaderProgramHandle rhs) const
	{
		return this->handle != rhs;
	}

	void ShaderProgram::verify() const
	{
		topaz_assert(this->handle != 0, "tz::gl::ShaderProgram::verify(): Verification Failed!");
	}

	void ShaderProgram::nullify_all()
	{
		// Can't fill it because noncopyable, so we just assign them all.
		for(auto& shader_ptr : this->shaders)
			shader_ptr = nullptr;
	}



	Shader::Shader(ShaderType type): Shader(type, std::string{}){}

	Shader::Shader(ShaderType type, std::string source): type(type), source(source), handle(glCreateShader(detail::resolve_type(this->type))), compilation_successful(false)
	{
		this->upload_source(this->source);
	}

	void Shader::upload_source(std::string source)
	{
		this->verify();
		this->compilation_successful = false;
		// Cache it.
		this->source = source;

		// Lets send it over!
		const GLchar* src = this->source.c_str();
		auto len = static_cast<GLint>(this->source.length());
		glShaderSource(this->handle, 1, &src, &len);
	}

	bool Shader::has_source() const
	{
		return !this->source.empty();
	}

	bool Shader::compiled() const
	{
		return this->compilation_successful;
	}

	void Shader::verify() const
	{
		topaz_assert(this->handle != 0, "tz::gl::Shader::verify(): Verification Failed!");
	}

	namespace detail
	{
		constexpr GLenum resolve_type(ShaderType type)
		{
			switch(type)
			{
				case ShaderType::Vertex:
					return GL_VERTEX_SHADER;
				case ShaderType::TessellationControl:
					return GL_TESS_CONTROL_SHADER;
				case ShaderType::TessellationEvaluation:
					return GL_TESS_EVALUATION_SHADER;
				case ShaderType::Geometry:
					return GL_GEOMETRY_SHADER;
				case ShaderType::Fragment:
					return GL_FRAGMENT_SHADER;
				case ShaderType::Compute:
					return GL_COMPUTE_SHADER;
			}
			return GL_INVALID_VALUE;
		}
	}

	namespace bound
	{
		ShaderProgramHandle shader_program()
		{
			int param;
			glGetIntegerv(GL_CURRENT_PROGRAM, &param);
			return static_cast<ShaderProgramHandle>(param);
		}
	}
}