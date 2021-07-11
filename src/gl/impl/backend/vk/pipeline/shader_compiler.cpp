#if TZ_VULKAN
#include "gl/impl/backend/vk/pipeline/shader_compiler.hpp"
#include <fstream>
#include <cstdint>

namespace tz::gl::vk
{
    std::optional<std::vector<char>> read_external_shader(std::string shader_name)
    {
        shader_name += ".spv";
        std::ifstream file{shader_name, std::ios::ate | std::ios::binary};

        if(!file.is_open())
        {
            return std::nullopt;
        }
        
        auto file_size_bytes = static_cast<std::size_t>(file.tellg());
        file.seekg(0);
        std::vector<char> buffer;
        buffer.resize(file_size_bytes);
        file.read(buffer.data(), file_size_bytes);
        file.close();
        return buffer;
    }
}

#endif