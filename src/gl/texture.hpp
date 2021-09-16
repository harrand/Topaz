#ifndef TOPAZ_GL_TEXTURE_HPP
#define TOPAZ_GL_TEXTURE_HPP
#include "core/assert.hpp"
#include "gl/declare/texture.hpp"
#include "stb_image.h"
#include <filesystem>
#include <span>
#include <vector>
#include <cstddef>

namespace tz::gl
{
    /**
     * @brief Represents image data for a texture.
     */
    struct TextureData
    {
        /**
         * @brief Retrieve image data from an existing image file.
         * Supports the following image file formats:
         * - TGA
         * - BMP
         * - PSD (composited view only)
         * - GIF (no animation support)
         * - HDR
         * - PIC
         * - PNM
         * @param image_path Path to an existing valid image file on the file system.
         * @param format Format of the output texture data. Doesn't have to match the image file - A conversion will occur in this case.
         * @return TextureData Containing the image data.
         */
        static TextureData from_image_file(const std::filesystem::path image_path, TextureFormat format);
        /**
         * @brief Retrieve image data from memory.
         * 
         * @param width Width of the image, in pixels.
         * @param height Height of the image, in pixels.
         * @param image_data Array-view of the entirety of the image data.
         * @return TextureData Containing the image data.
         */
        static TextureData from_memory(unsigned int width, unsigned int height, std::span<const unsigned char> image_data);
        /**
         * @brief Create texture-data of unspecified values.
         * 
         * @param width Width of the image, in pixels.
         * @param height Height of the image, in pixels.
         * @param format Format of the output texture data.
         * @return TextureData Containing the image data.
         */
        static TextureData uninitialised(unsigned int width, unsigned int height, TextureFormat format);

        unsigned int width;
        unsigned int height;
        std::vector<std::byte> image_data;
    };
}

#endif // TOPAZ_GL_TEXTURE_HPP