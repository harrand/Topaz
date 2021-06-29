#ifndef TOPAZ_GL_TEXTURE_HPP
#define TOPAZ_GL_TEXTURE_HPP
#include "core/assert.hpp"
#include "gl/impl/common/resource.hpp"
#include "stb_image.h"
#include <filesystem>
#include <cstring>
#include <span>

namespace tz::gl
{
    struct TextureData
    {
        static TextureData from_image_file(const std::filesystem::path image_path, TextureFormat format);
        static TextureData from_memory(unsigned int width, unsigned int height, std::span<const unsigned char> image_data);
        static TextureData uninitialised(unsigned int width, unsigned int height, TextureFormat format);

        unsigned int width;
        unsigned int height;
        std::vector<std::byte> image_data;
    };
}

#endif // TOPAZ_GL_TEXTURE_HPP