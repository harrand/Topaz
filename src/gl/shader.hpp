#ifndef TOPAZ_GL_SHADER_HPP
#define TOPAZ_GL_SHADER_HPP
#include "glad/glad.h"
#include <cstdint>
#include <array>
#include <memory>
#include <string>

namespace tz::gl
{
	enum class ShaderType : std::size_t
	{
		Vertex,
		TessellationControl,
		TessellationEvaluation,
		Geometry,
		Fragment,

		NUM_TYPES,
		Compute
	};

	class Shader;

	using ShaderProgramHandle = GLuint;

	class ShaderProgram
	{
	public:
		ShaderProgram();
		ShaderProgram(const ShaderProgram& copy) = delete;
		ShaderProgram(ShaderProgram&& move);
		~ShaderProgram();
		ShaderProgram& operator=(const ShaderProgram& rhs) = delete;
		ShaderProgram& operator=(ShaderProgram&& rhs);

		template<ShaderType Type>
		void set(std::unique_ptr<Shader> shader);
		void set(ShaderType type, std::unique_ptr<Shader> shader);
		template<typename... Args>
		Shader* emplace(ShaderType type, Args&&... args);
		bool linkable() const;
		bool usable() const;
		void bind();

		bool has_shader(ShaderType type) const;
		bool operator==(ShaderProgramHandle rhs) const;
		bool operator!=(ShaderProgramHandle rhs) const;

		friend class ShaderCompiler; // Pretty much unavoidable tight-coupling, and I don't want to merge the two things.
	private:
		void verify() const;
		void nullify_all();

		ShaderProgramHandle handle;
		std::array<std::unique_ptr<Shader>, static_cast<std::size_t>(ShaderType::NUM_TYPES)> shaders;
		bool ready;
	};

	class Shader
	{
	public:
		Shader(ShaderType type);
		Shader(ShaderType type, std::string source);
		void upload_source(std::string source);
		bool has_source() const;
		bool compiled() const;

		friend class ShaderCompiler; // Pretty much unavoidable tight-coupling, and I don't want to merge the two things.
		friend class ShaderProgram; // Same here. Hard to attach via our handle if we can't access it, and exposing the handle is not going to happen.
	private:
		void verify() const;

		ShaderType type;
		std::string source;
		GLuint handle;
		bool compilation_successful; // We always expect ShaderCompiler to mess with this.
	};

	namespace detail
	{
		constexpr GLenum resolve_type(ShaderType type);
	}

	namespace bound
	{
		ShaderProgramHandle shader_program();
	}
}

#include "gl/shader.inl"
#endif // TOPAZ_GL_SHADER_HPP