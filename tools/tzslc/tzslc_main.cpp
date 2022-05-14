#include "core/assert.hpp"
#include "preprocessor.hpp"
#include <cstdio>
#include <fstream>
#include <cerrno>
#include <cstring>

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
			output = fopen(arg_next.data(), "w");
			tz_assert(output != nullptr, "Failed to open output stream. Perhaps missing intermediate directory, or no write permissions for this area of the filesystem?\nOutput was %s.\nErrno says: %s", arg_next.data(), std::strerror(errno));
		}
	}
	return output;
}

int main(int argc, char** argv)
{
	tz_assert(argc >= 2, "Not enough arguments (%d). At least 2. Usage: `tzslc <tzsl_file_path> [-o <output_file_path>]`", argc);
	const char* tzsl_filename = argv[1];

	FILE* out = get_output_stream(argc, argv);
	{
		std::ifstream shader{tzsl_filename, std::ios::ate | std::ios::binary};
		tz_assert(shader.is_open(), "Cannot open shader file %s", tzsl_filename);
		auto file_size_bytes = static_cast<std::size_t>(shader.tellg());
		shader.seekg(0);
		std::string buffer;
		buffer.resize(file_size_bytes);
		shader.read(buffer.data(), file_size_bytes);
		shader.close();

		tzslc::preprocess(buffer);
		for(char c : buffer)
		{
			std::fprintf(out, "%c", c);
		}
	}
	if(out != stdout)
	{
		fclose(out);
	}
}
