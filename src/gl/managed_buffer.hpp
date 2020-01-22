#ifndef TOPAZ_GL_MANAGED_BUFFER_HPP
#define TOPAZ_GL_MANAGED_BUFFER_HPP
#include "gl/buffer.hpp"
#include "memory/block.hpp"
#include <map>
#include <optional>

// Forward declares
namespace tz::gl
{
    class Object;
}

namespace tz::gl
{
    struct ManagedBufferRegion
    {
        void* mapping_begin = nullptr;
        const char* region_name = "<Unknown Region>";
        tz::mem::Block block = {nullptr, nullptr};
        
        bool operator<(const ManagedBufferRegion& rhs) const;
    };

    class IManagedBuffer
    {
    public:
        virtual ~IManagedBuffer() = default;
        virtual ManagedBufferRegion region(std::size_t offset_bytes, std::size_t size_bytes, std::string name) = 0;
        virtual void erase(const std::string& region_name) = 0;
        virtual bool defragment() = 0;
        virtual std::size_t regions_usage() const = 0;
        virtual bool regions_full() const = 0;
    };

    template<tz::gl::BufferType Type>
    class ManagedBuffer : public IManagedBuffer, public Buffer<Type>
    {
    public:
        ManagedBuffer(tz::gl::Object& holder);
        virtual ManagedBufferRegion region(std::size_t offset_bytes, std::size_t size_bytes, std::string name) override;
        virtual void erase(const std::string& region_name) override;
        virtual bool defragment() override;
        virtual std::size_t regions_usage() const override;
        virtual bool regions_full() const override;

        virtual tz::mem::Block map(MappingPurpose purpose = MappingPurpose::ReadWrite) override;
        virtual void unmap() override;
        const ManagedBufferRegion& operator[](const std::string& name) const;
    private:
        using MapType = std::map<ManagedBufferRegion, std::string>;
        void verify_mapped() const;
        bool relocate_region(const std::string& region_name, std::size_t byte_index);
        const char* region_within(std::size_t byte_index) const;
        MapType::iterator find_region_iter(const std::string& name);
        MapType::const_iterator find_region_iter(const std::string& name) const;

        tz::gl::Object& holder;
        MapType regions;
        std::optional<tz::mem::Block> mapped_block;
    };

    using ManagedVBO = ManagedBuffer<tz::gl::BufferType::Array>;
    using ManagedIBO = ManagedBuffer<tz::gl::BufferType::Index>;
    using ManagedUBO = ManagedBuffer<tz::gl::BufferType::UniformStorage>;
    using ManagedSSBO = ManagedBuffer<tz::gl::BufferType::ShaderStorage>;
}

#include "gl/managed_buffer.inl"
#endif // TOPAZ_GL_MANAGED_BUFFER_HPP