#ifndef TOPAZ_GL_RESOURCE_HPP
#define TOPAZ_GL_RESOURCE_HPP
#include "core/assert.hpp"
#include "gl/api/resource.hpp"
#include "gl/buffer.hpp"
#include "gl/texture.hpp"
#include <cstring>

namespace tz::gl
{
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