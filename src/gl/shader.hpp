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
		bool usable() const;
		bool has_shader(ShaderType type) const;
	private:
		void verify() const;
		void nullify_all();

		GLuint handle;
		std::array<std::unique_ptr<Shader>, static_cast<std::size_t>(ShaderType::NUM_TYPES)> shaders;
	};

	class Shader
	{
	public:
		Shader(ShaderType type);
		Shader(ShaderType type, std::string source);
		void upload_source(std::string source);
		bool has_source() const;

		friend class ShaderCompiler; // Pretty much unavoidable tight-coupling, and I don't want to merge the two things.
	private:
		void verify() const;

		ShaderType type;
		std::string source;
		GLuint handle;
	};

	namespace detail
	{
		constexpr GLenum resolve_type(ShaderType type);
	}
}

#include "gl/shader.inl"
#endif // TOPAZ_GL_SHADER_HPP