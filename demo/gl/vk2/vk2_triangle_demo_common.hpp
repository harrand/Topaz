#include "core/assert.hpp"
#include <cstddef>
#include <fstream>
#include <vector>

constexpr static std::size_t max_frames_in_flight = 2;

inline std::vector<char> read_shader_code(const char* relative_shader_filename)
{
	 std::ifstream file(relative_shader_filename, std::ios::ate | std::ios::binary);

	 tz_assert(file.is_open(), "Failed to find shader file %s. Are you running in the correct working directory?", relative_shader_filename);

        size_t fileSize = (size_t) file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;
}

