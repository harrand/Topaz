#include "core/assert.hpp"
#include "preprocessor.hpp"
#include <cstdio>
#include <fstream>
#include <filesystem>

PreprocessorModuleField selected_modules(int argc, char** argv)
{
	PreprocessorModuleField modules;
	for(std::size_t i = 0; i < argc; i++)
	{
		std::string_view arg{argv[i]};
		if(arg.starts_with("-m"))
		{
			arg.remove_prefix(2);
			// TODO: Check module names
			for(int i = static_cast<int>(PreprocessorModule::Begin); i < static_cast<int>(PreprocessorModule::End); i++)
			{
				const char* module_name = tzslc::preprocessor_module_names[i];
				if(arg == module_name)
				{
					modules |= static_cast<PreprocessorModule>(i);
				}
			}
			if(arg == "all")
			{
				return PreprocessorModuleField::All();
			}
		}
	}
	return modules;
}

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
		}
	}
	return output;
}

FILE* get_output_stream_meta(int argc, char** argv)
{
	// get_output_stream() returns either stdout or a specific file.
	// Often we will need to output a .meta file along with the compiled output. If we have a specific file, we chuck the meta in the same output directory as the output file.
	// If we're writing to stdout, we won't bother writing the meta at all
	for(std::size_t i = 0; i < argc - 1; i++)
	{
		std::string_view arg{argv[i]};
		std::string_view arg_next{argv[i + 1]};
		if(arg == "-o")
		{
			// arg_next is the output file name. Retrieve the parent directory
			std::filesystem::path output_path = arg_next;
			output_path += ".meta";
			return fopen(output_path.string().c_str(), "w");
		}
	}
	return nullptr;
}

int main(int argc, char** argv)
{
	tz_assert(argc >= 2, "Not enough arguments (%d). At least 2", argc);
	const char* glsl_filename = argv[1];
	PreprocessorModuleField modules = selected_modules(argc - 2, argv + 2);

	FILE* out = get_output_stream(argc, argv);
	FILE* out_meta = get_output_stream_meta(argc, argv);
	{
		std::ifstream shader{glsl_filename, std::ios::ate | std::ios::binary};
		tz_assert(shader.is_open(), "Cannot open shader file %s", glsl_filename);
		auto file_size_bytes = static_cast<std::size_t>(shader.tellg());
		shader.seekg(0);
		std::string buffer;
		buffer.resize(file_size_bytes);
		shader.read(buffer.data(), file_size_bytes);
		shader.close();

		std::string metadata;
		tzslc::preprocess(modules, buffer, metadata);
		std::fprintf(out, "%s", buffer.data());

		if(out_meta != nullptr)
		{
			std::fprintf(out_meta, "%s", metadata.data());
		}
	}
	if(out != stdout)
	{
		fclose(out);
	}
	if(out_meta != nullptr)
	{
		fclose(out_meta);
	}
}