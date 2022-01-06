#ifndef TZSLC_SHADER_EXPORT_HPP
#define TZSLC_SHADER_EXPORT_HPP
#include <string>
#include <string_view>

namespace tzslc
{
	bool export_header(std::string_view filename, std::string& buffer);
}

#endif // TZSLC_SHADER_EXPORT_HPP
