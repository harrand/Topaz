#ifndef TOPAZ_GL_RESOURCE_HPP
#define TOPAZ_GL_RESOURCE_HPP
#include "core/assert.hpp"
#include "gl/api/resource.hpp"
#include "stb_image.h"
#include <cstring>
#include <vector>
#include <filesystem>

namespace tz::gl
{
    struct BufferData
    {
        template<typename T>
        static BufferData FromArray(std::span<const T> data)
        {
            BufferData buf;
            buf.data.resize(data.size_bytes());
            std::memcpy(buf.data.data(), data.data(), data.size_bytes());
            return buf;
        }

        template<typename T>
        static BufferData FromValue(const T& data)
        {
            BufferData buf;
            buf.data.resize(sizeof(T));
            std::memcpy(buf.data.data(), &data, sizeof(T));
            return buf;
        }
        std::vector<std::byte> data;
    };

    /**
     * @brief Renderer Resource representing a uniform buffer.
     * 
     */
    class BufferResource : public IResourceCopyable<BufferResource>
    {
    public:
        BufferResource(BufferData data):
        data(data)
        {}

        virtual constexpr ResourceType get_type() const final
        {
            return ResourceType::Buffer;
        }

        virtual std::span<const std::byte> get_resource_bytes() const final
        {
            return {this->data.data.begin(), this->data.data.end()};
        }
    private:
        BufferData data;
    };

    class DynamicBufferResource : public IDynamicResourceCopyable<DynamicBufferResource>
    {
    public:
        DynamicBufferResource(BufferData data):
        initial_data(data),
        resource_data(nullptr)
        {}

        virtual constexpr ResourceType get_type() const final
        {
            return ResourceType::Buffer;
        }

        virtual std::span<const std::byte> get_resource_bytes() const final
        {
            if(this->resource_data == nullptr)
            {
                return {this->initial_data.data.begin(), this->initial_data.data.end()};
            }
            return {this->resource_data, this->resource_data + this->initial_data.data.size()};
        }

        virtual std::span<std::byte> get_resource_bytes_dynamic() final
        {
            if(this->resource_data == nullptr)
            {
                return {this->initial_data.data.begin(), this->initial_data.data.end()};
            }
            return {this->resource_data, this->resource_data + this->initial_data.data.size()};
        }

        virtual void set_resource_data(std::byte* resource_data)
        {
            auto res_data = this->get_resource_bytes();
            this->resource_data = resource_data;
            std::memcpy(this->resource_data, res_data.data(), res_data.size_bytes());
        }
    private:
        BufferData initial_data;
        std::byte* resource_data;
    };

    struct TextureData
    {
        static TextureData FromImageFile(const std::filesystem::path image_path, TextureFormat format)
        {
            TextureData data;
            int components_per_element;
            switch(format)
            {
                case TextureFormat::Rgba32Signed:
                [[fallthrough]];
                case TextureFormat::Rgba32Unsigned:
                [[fallthrough]];
                case TextureFormat::Rgba32sRGB:
                    components_per_element = 4;
                break;
                case TextureFormat::DepthFloat32:
                    components_per_element = 1;
                break;
                default:
                    tz_error("No support for given TextureFormat");
                break;
            }

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

        static TextureData FromMemory(unsigned int width, unsigned int height, std::span<const unsigned char> image_data)
        {
            TextureData data;
            data.width = width;
            data.height = height;
            data.image_data.resize(image_data.size_bytes());
            std::memcpy(data.image_data.data(), image_data.data(), image_data.size_bytes());
            return data;
        }

        static TextureData Uninitialised(unsigned int width, unsigned int height, TextureFormat format)
        {
            TextureData data;
            data.width = width;
            data.height = height;
            std::size_t element_size;
            switch(format)
            {
                case TextureFormat::Rgba32Signed:
                [[fallthrough]];
                case TextureFormat::Rgba32Unsigned:
                [[fallthrough]];
                case TextureFormat::Rgba32sRGB:
                    element_size = 4;
                break;
                case TextureFormat::DepthFloat32:
                    element_size = 4;
                break;
                default:
                    tz_error("No support for given TextureFormat");
                break;
            }
            data.image_data.resize(element_size * width * height);
            return data;
        }

        unsigned int width;
        unsigned int height;
        std::vector<std::byte> image_data;
    };

    class TextureResource : public IResourceCopyable<TextureResource>
    {
    public:
        TextureResource(TextureData data, TextureFormat format, TextureProperties properties = TextureProperties::get_default()):
        data(data),
        format(format),
        properties(properties){}

        virtual constexpr ResourceType get_type() const final
        {
            return ResourceType::Texture;
        }

        virtual std::span<const std::byte> get_resource_bytes() const final
        {
            return {this->data.image_data.begin(), this->data.image_data.end()};
        }

        const TextureFormat& get_format() const
        {
            return this->format;
        }

        const TextureProperties& get_properties() const
        {
            return this->properties;
        }

        unsigned int get_width() const
        {
            return data.width;
        }

        unsigned int get_height() const
        {
            return data.height;
        }
    private:
        TextureData data;
        TextureFormat format;
        TextureProperties properties;
    };
}

#endif // TOPAZ_GL_DEVICE_HPP