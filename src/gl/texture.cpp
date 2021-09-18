#include "gl/texture.hpp"
#include <cstring>

namespace tz::gl
{
    constexpr std::size_t component_count(TextureFormat fmt)
    {
        switch(fmt)
        {
            case TextureFormat::Rgba32Signed:
            [[fallthrough]];
            case TextureFormat::Rgba32Unsigned:
            [[fallthrough]];
            case TextureFormat::Rgba32sRGB:
                return 4;
            break;
            case TextureFormat::DepthFloat32:
                return 1;
            break;
            default:
                tz_error("No support for given TextureFormat");
                return 0;
            break;
        }
    }

    TextureData TextureData::from_image_file(const std::filesystem::path image_path, TextureFormat format)
    {
        TextureData data;
        int components_per_element = component_count(format);
        {
            int w, h;
            int channels_in_file;
            unsigned char* imgdata = stbi_load(image_path.string().c_str(), &w, &h, &channels_in_file, components_per_element);
            data.width = w;
            data.height = h;
            std::size_t imgdata_size = w * h * components_per_element;
            data.image_data.resize(imgdata_size);
            std::memcpy(data.image_data.data(), imgdata, imgdata_size);
        }
        return data;
    }

    TextureData TextureData::from_memory(unsigned int width, unsigned int height, std::span<const unsigned char> image_data)
    {
        TextureData data;
        data.width = width;
        data.height = height;
        data.image_data.resize(image_data.size_bytes());
        std::memcpy(data.image_data.data(), image_data.data(), image_data.size_bytes());
        return data;
    }

    TextureData TextureData::uninitialised(unsigned int width, unsigned int height, TextureFormat format)
    {
        TextureData data;
        data.width = width;
        data.height = height;
        std::size_t element_size = component_count(format);
        data.image_data.resize(element_size * width * height);
        return data;
    }
}