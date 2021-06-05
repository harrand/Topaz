#include "core/assert.hpp"
#include "preprocessor.hpp"
#include <cstdio>
#include <fstream>

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

int main(int argc, char** argv)
{
    tz_assert(argc >= 2, "Not enough arguments (%d). At least 2", argc);
    const char* glsl_filename = argv[1];
    PreprocessorModuleField modules = selected_modules(argc - 2, argv + 2);

    FILE* out = get_output_stream(argc, argv);
    {
        std::ifstream shader{glsl_filename, std::ios::ate | std::ios::binary};
        tz_assert(shader.is_open(), "Cannot open shader file %s", glsl_filename);
        auto file_size_bytes = static_cast<std::size_t>(shader.tellg());
        shader.seekg(0);
        std::string buffer;
        buffer.resize(file_size_bytes);
        shader.read(buffer.data(), file_size_bytes);
        shader.close();

        tzslc::preprocess(modules, buffer);
        std::fprintf(out, "%s", buffer.data());
    }
    if(out != stdout)
    {
        fclose(out);
    }
}