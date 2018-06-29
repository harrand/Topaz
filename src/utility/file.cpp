#include "file.hpp"
#include <sstream>
#include <fstream>

namespace tz::utility::file
{
    std::string read(const std::string& path)
    {
        std::stringstream string_stream;
        string_stream << std::ifstream(path.c_str()).rdbuf();
        return string_stream.str();
    }
}