#include "core/core.hpp"
#include "core/resource_manager.hpp"
#include "core/debug/assert.hpp"
#include "stb_image_write.h"
#include <algorithm>

namespace tz::ext::stb
{
    enum class ImageWriteType
    {
        PNG,
        BMP,
        TGA,
        JPG,
        Unknown
    };

    ImageWriteType parse_extension(std::string extension)
    {
        // Make sure extension is lower-case.
        std::transform(extension.begin(), extension.end(), extension.begin(),
    [](unsigned char c){ return std::tolower(c); });
        if(extension == "png")
        {
            return ImageWriteType::PNG;
        }
        else if(extension == "bmp")
        {
            return ImageWriteType::BMP;
        }
        else if(extension == "tga")
        {
            return ImageWriteType::TGA;
        }
        else if(extension == "jpg")
        {
            return ImageWriteType::JPG;
        }
        return ImageWriteType::Unknown;
    }

    template<class PixelType>
    std::string write_image(const char* path, const tz::gl::Image<PixelType>& image, std::string name)
    {
        std::string result_path = tz::core::res().get_path();
        topaz_assert(!result_path.empty(), "tz::ext::stb::write_image(", path, ", image, ", name, "): Invalid resource manager path \"", tz::core::res().get_path(), "\"");
        result_path += path;
        if(result_path.back() != '/' || result_path.back() != '\\')
            result_path += '/';
        result_path += name;
        std::size_t dot_position = name.find_last_of('.');
        topaz_assert(dot_position != std::string::npos, "tz::ext::stb::write_image(", path, ", image, ", name, "): Resultant filename \"", name, "\" did not have a valid extension.");
        std::string extension = name.substr(dot_position + 1);
        ImageWriteType type = parse_extension(extension);
        topaz_assert(type != ImageWriteType::Unknown, "tz::ext::stb::write_image(", path, ", image, ", name, "): Failed to parse file extension of filename \"", name, "\"");

        // Write the damn image.
        switch(type)
        {
            case ImageWriteType::PNG:
                stbi_write_png(result_path.c_str(), image.get_width(), image.get_height(), PixelType::num_components, image.data(), sizeof(std::byte) * PixelType::num_components * image.get_width());
            break;
            case ImageWriteType::BMP:
                stbi_write_bmp(result_path.c_str(), image.get_width(), image.get_height(), PixelType::num_components, image.data());
            break;
            case ImageWriteType::TGA:
                stbi_write_tga(result_path.c_str(), image.get_width(), image.get_height(), PixelType::num_components, image.data());
            break;
            case ImageWriteType::JPG:
                // JPEG does ignore alpha channels in input data; quality is between 1 and 100. Higher quality looks better but results in a bigger image.
                constexpr int jpg_quality = 75;
                stbi_write_jpg(result_path.c_str(), image.get_width(), image.get_height(), PixelType::num_components, image.data(), jpg_quality);
            break;
        }

        return result_path;
    }
}