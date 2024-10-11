#include "assert.hpp"
#include "tzsl.hpp"
#include <cstdio>
#include <fstream>
#include <cerrno>
#include <cstring>
#include <filesystem>

FILE* get_output_stream(int argc, char** argv)
{
	// We're either writing to stdout or a specific file. Return the relevant filestream
	FILE* output = stdout;
	for(std::size_t i = 0; i < argc - 1; i++)
	{
		std::string_view arg{argv[i]};
		std::string_view arg_next{argv[i + 1]};
		if(arg == "-o")
		{
			std::filesystem::path out_path{arg_next.data()};
			std::filesystem::path parent = out_path.parent_path();
			if(!std::filesystem::exists(parent))
			{
				std::filesystem::create_directories(parent);
			}
			output = fopen(arg_next.data(), "w");
			tzslc_assert(output != nullptr, "Failed to open output stream. Perhaps missing intermediate directory, or no write permissions for this area of the filesystem?\nOutput was %s.\nErrno says: %s", arg_next.data(), std::strerror(errno));
		}
	}
	return output;
}

tzslc::GLSLDialect get_dialect(int argc, char** argv)
{
	for(std::size_t i = 0; i < argc - 1; i++)
	{
		std::string_view arg{argv[i]};
		std::string_view arg_next{argv[i + 1]};
		if(arg == "-api")
		{
			if(arg_next == "vk")
			{
				return tzslc::GLSLDialect::Vulkan;
			}
			else if(arg_next == "ogl")
			{
				return tzslc::GLSLDialect::OpenGL;
			}
			else
			{
				tzslc_error("Unrecognised -api \"%s\". Expected 'vk' or 'ogl'", arg_next.data());
				return tzslc::GLSLDialect::Vulkan;
			}
		}
	}
	tzslc_error("Missing -api invocation. Must specify -api vk, or -api ogl");
	return tzslc::GLSLDialect::Vulkan;
}

tzslc::BuildConfig get_config(int argc, char** argv)
{
	for(std::size_t i = 0; i < argc; i++)
	{
		std::string_view arg{argv[i]};
		if(arg == "-g")
		{
			return tzslc::BuildConfig::Debug;
		}
	}
	return tzslc::BuildConfig::Release;
}

int main(int argc, char** argv)
{
	tzslc_assert(argc >= 4, "Not enough arguments (%d). At least 4. Usage: `tzslc <tzsl_file_path> <-api vk/ogl>[-o <output_file_path>]`", argc);
	const char* tzsl_filename = argv[1];

	FILE* out = get_output_stream(argc, argv);
	tzslc::GLSLDialect dialect = get_dialect(argc, argv);
	tzslc::BuildConfig build_config = get_config(argc, argv);
	{
		std::ifstream shader{tzsl_filename, std::ios::ate | std::ios::binary};
		tzslc_assert(shader.is_open(), "Cannot open shader file %s", tzsl_filename);
		auto file_size_bytes = static_cast<std::size_t>(shader.tellg());
		shader.seekg(0);
		std::string buffer;
		buffer.resize(file_size_bytes);
		shader.read(buffer.data(), file_size_bytes);
		shader.close();

		tzslc::compile_to_glsl(buffer, tzsl_filename, dialect, build_config);
		for(char c : buffer)
		{
			if(c == '\r') continue;
			std::fprintf(out, "%c", c);
		}
	}
	if(out != stdout)
	{
		fclose(out);
	}
}
