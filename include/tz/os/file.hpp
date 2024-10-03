#ifndef TOPAZ_OS_FILE_HPP
#define TOPAZ_OS_FILE_HPP
#include "tz/core/error.hpp"
#include <string>
#include <filesystem>
#include <expected>

namespace tz::os
{
	std::expected<std::string, tz::error_code> read_file(std::filesystem::path path);
}

#endif // TOPAZ_OS_FILE_HPP