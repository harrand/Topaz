#include "gl/shader.hpp"
#include "gl/texture.hpp"
#include "core/debug/assert.hpp"
#include <algorithm>

namespace tz::gl
{
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
	
	ShaderProgram::ShaderProgram(): handle(glCreateProgram()), shaders(), textures(), ready(false)
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
				if(child_ptr.has_value())
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
			if(shader_ptr.has_value())
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
		this->bind_textures();
	}

	std::size_t ShaderProgram::attached_textures_capacity() const
	{
		GLint max_textures_accessed_by_fragment_shader;
		glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &max_textures_accessed_by_fragment_shader);
		return std::min(max_textures_accessed_by_fragment_shader, GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS);
	}

	std::size_t ShaderProgram::attached_textures_size() const
	{
		std::size_t num = 0;
		for(const Texture* tex : this->textures)
			if(tex != nullptr)
				num++;
		return num;
	}

	void ShaderProgram::attach_texture(std::size_t idx, const Texture* texture, std::string sampler_name)
	{
		topaz_assert(idx < GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, "tz::gl::ShaderProgram::attach_texture(", idx, ", const Texture*, ", sampler_name, "): Index was out of range. Max: ", GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS);
		this->textures[idx] = texture;
		this->texture_names[idx] = sampler_name;
	}

	void ShaderProgram::detach_texture(std::size_t idx)
	{
		topaz_assert(idx < GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, "tz::gl::ShaderProgram::detach_texture(", idx, "): Index was out of range. Max: ", GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS);
		this->textures[idx] = nullptr;
	}

	const Texture* ShaderProgram::get_attachment(std::size_t idx) const
	{
		topaz_assert(idx < GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, "tz::gl::ShaderProgram::detach_texture(", idx, "): Index was out of range. Max: ", GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS);
		return this->textures[idx];
	}

	bool ShaderProgram::has_shader(ShaderType type) const
	{
		topaz_assert(type != ShaderType::NUM_TYPES, "tz::gl::ShaderProgram::has_shader(...): Was given type index ", static_cast<std::size_t>(type), " (ShaderTypes::NUM_TYPES) which is not a legal parameter here.");
		return this->shaders[static_cast<std::size_t>(type)].has_value();
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
			shader_ptr = std::nullopt;
	}

	void ShaderProgram::bind_textures() const
	{
		for(std::size_t i = 0; i < GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS; i++)
		{
			const Texture* tex = this->textures[i];
			const char* tex_name = this->texture_names[i].c_str();
			if(tex != nullptr)
			{
				tex->bind(i);
				GLint sampler_location = glGetUniformLocation(this->handle, tex_name);
				glUniform1i(sampler_location, i);
			}
		}
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