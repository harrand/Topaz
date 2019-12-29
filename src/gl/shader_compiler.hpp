#ifndef TOPAZ_GL_SHADER_COMPILER_HPP
#define TOPAZ_GL_SHADER_COMPILER_HPP
#include <string>

namespace tz::gl
{
	// Forward declares
	class Shader;

	enum class ShaderCompilerType
	{
		Debug,
		Release,
		Auto
	};

	struct ShaderCompilerOptions
	{
		ShaderCompilerType type;
	};

	namespace detail
	{
		constexpr ShaderCompilerOptions get_default_compiler_options()
		{
			return {ShaderCompilerType::Auto};
		}
	}

	class ShaderCompilerDiagnostic
	{
	public:
		ShaderCompilerDiagnostic(bool success, std::string info_log);
		bool successful() const;
		const std::string& get_info_log() const;
	private:
		bool success;
		std::string info_log;
	};

	class ShaderCompiler
	{
	public:
		ShaderCompiler(ShaderCompilerOptions options = detail::get_default_compiler_options());
		ShaderCompilerDiagnostic compile(const Shader& shader) const;
	private:
		ShaderCompilerOptions options;
	};
}

#endif // TOPAZ_GL_SHADER_COMPILER_HPP