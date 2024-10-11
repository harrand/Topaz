#ifndef TZSLC_PREPROCESSOR_HPP
#define TZSLC_PREPROCESSOR_HPP
#include <array>
#include <string>
#include <filesystem>

namespace tzslc
{
	enum class GLSLDialect
	{
		Vulkan,
		OpenGL
	};

	enum class BuildConfig
	{
		Debug,
		Release,
	};

	void compile_to_glsl(std::string& shader_source, std::filesystem::path shader_filename, GLSLDialect dialect, BuildConfig build_config);
}

#endif // TZSLC_PREPROCESSOR_HPP
