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
        BufferResource(BufferData data);

        virtual constexpr ResourceType get_type() const final
        {
            return ResourceType::Buffer;
        }

        virtual std::span<const std::byte> get_resource_bytes() const final;
    private:
        BufferData data;
    };

    class DynamicBufferResource : public IDynamicResourceCopyable<DynamicBufferResource>
    {
    public:
        DynamicBufferResource(BufferData data);

        virtual constexpr ResourceType get_type() const final
        {
            return ResourceType::Buffer;
        }

        virtual std::span<const std::byte> get_resource_bytes() const final;
        virtual std::span<std::byte> get_resource_bytes_dynamic() final;
        virtual void set_resource_data(std::byte* resource_data);
    private:
        BufferData initial_data;
        std::byte* resource_data;
    };

    class TextureResource : public IResourceCopyable<TextureResource>
    {
    public:
        TextureResource(TextureData data, TextureFormat format, TextureProperties properties = TextureProperties::get_default());

        virtual constexpr ResourceType get_type() const final
        {
            return ResourceType::Texture;
        }

        virtual std::span<const std::byte> get_resource_bytes() const final;
        
        const TextureFormat& get_format() const;
        const TextureProperties& get_properties() const;

        unsigned int get_width() const;
        unsigned int get_height() const;
    private:
        TextureData data;
        TextureFormat format;
        TextureProperties properties;
    };
}

#endif // TOPAZ_GL_DEVICE_HPP