#include "tzglp.hpp"
#include "core/core.hpp"
#include <string>
#include <iostream>
#include <string_view>
#include <optional>
#include <fstream>
#include <sstream>

using MaybeArgs = std::optional<PreprocessorArgs>;

void print_usages();
MaybeArgs option_a(std::string_view modules_list, std::string_view filename);
MaybeArgs option_b(std::string_view modules_list);
MaybeArgs option_c(std::string_view modules_list, std::string_view filename, std::string_view include_paths);
MaybeArgs option_d(std::string_view modules_list, std::string_view include_paths);

std::vector<std::string> split(const std::string& i_str, const std::string& i_delim)
{
    std::vector<std::string> result;
    
    std::size_t found = i_str.find(i_delim);
    std::size_t startIndex = 0;

    while(found != std::string::npos)
    {
        result.push_back(std::string(i_str.begin()+startIndex, i_str.begin()+found));
        startIndex = found + i_delim.size();
        found = i_str.find(i_delim, startIndex);
    }
    if(startIndex != i_str.size())
        result.push_back(std::string(i_str.begin()+startIndex, i_str.end()));
    return result;      
}

MaybeArgs parse_args(int argc, char** argv)
{
    // Option A: Preprocess an existing glsl file.
    // tzglp <--modules> <m1>[,m2,m3,m4,...] <filename>
    // Option B: Preprocess glsl source via stdin (filename omitted)
    // tzglp <--modules> <m1>[,m2,m3,m4,...]
    // Option C: Preprocess an existing glsl file with extra include paths.
    // tzglp <--modules> <m1>[,m2,m3,m4,...] [-I <dir1>] <filename>
    // Option D: Preprocess glsl source via stdin (filename omitted) with extra include paths.
    // tzglp <--modules> <m1>[,m2,m3,m4,...] [-I <dir1>]
    switch(argc)
    {
        case 4:
        {
            const char* modules_list = argv[2];
            const char* filename = argv[3];
            return option_a(modules_list, filename);
        }
        case 3:
        {
            const char* modules_list = argv[2];
            return option_b(modules_list);
        }
        case 6:
        {
            const char* modules_list = argv[2];
            const char* include_path = argv[4];
            const char* filename = argv[5];
            return option_c(modules_list, filename, include_path);
        }
        case 5:
        {
            const char* modules_list = argv[2];
            const char* include_path = argv[4];
            return option_d(modules_list, include_path);
        }
        default:
            print_error("error: invalid number of arguments specified (%d)", argc - 1);
            print_usages();
    }
    return {std::nullopt};
}

MaybeArgs option_d(std::string_view modules_list, std::string_view include_paths)
{
    std::string list_cpy{modules_list};
    std::string src;
    for (std::string line; std::getline(std::cin, line);)
    {
        src += line + "\n";
    }
    return PreprocessorArgs{split(list_cpy, ","), src, std::string{include_paths}};
}

MaybeArgs option_c(std::string_view modules_list, std::string_view filename, std::string_view include_paths)
{
    std::string list_cpy{modules_list};
    
    std::ifstream shader_file{filename.data()};
    if(!shader_file.good())
    {
        print_error("error: failed to read shader source file \"%s\"", filename.data());
        return {std::nullopt};
    }
    std::stringstream shader_src_buffer;
    shader_src_buffer << shader_file.rdbuf();

    return PreprocessorArgs{split(list_cpy, ","), shader_src_buffer.str(), std::string{include_paths}};
}

MaybeArgs option_b(std::string_view modules_list)
{
    return option_d(modules_list, "");
}

MaybeArgs option_a(std::string_view modules_list, std::string_view filename)
{
    return option_c(modules_list, filename, "");
}

int init(int argc, char** argv)
{
    std::optional<PreprocessorArgs> args = parse_args(argc, argv);
    if(!args.has_value())
    {
        return -1;
    }
    TZGLP tzglp{args.value()};
    if(tzglp.error())
    {
        return -1;
    }
    std::cout << tzglp.get_result();
    return 0;
}

int main(int argc, char** argv)
{
    int ret;
    tz::initialise("TZGLP");
    {
        ret = init(argc, argv);
    }
    tz::terminate();
    return ret;
}

void print_usages()
{
    constexpr char usages_str[] = R"S(
    note: usages are as follows:
    "tzglp <--modules> <m1>[,m2,m3,m4,...] <filename>" - from source file, no includes
    "tzglp <--modules> <m1>[,m2,m3,m4,...]" - from source file, no includes
    "tzglp <--modules> <m1>[,m2,m3,m4,...] [-I <dir>] <filename>" - from source file, includes
    "tzglp <--modules> <m1>[,m2,m3,m4,...] [-I <dir>]" - from stdin, includes
    )S";
    std::cout << usages_str;
}