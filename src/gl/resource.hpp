#ifndef TOPAZ_GL_RESOURCE_HPP
#define TOPAZ_GL_RESOURCE_HPP
#include "gl/api/resource.hpp"
#include <cstring>
#include <vector>

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
}

#endif // TOPAZ_GL_DEVICE_HPP