#ifndef TOPAZ_GL_MANAGED_BUFFER_HPP
#define TOPAZ_GL_MANAGED_BUFFER_HPP
#include "gl/buffer.hpp"
#include "memory/block.hpp"
#include "memory/demap.hpp"
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
        std::size_t offset;
        std::size_t size_bytes;

        bool operator<(const ManagedBufferRegion& rhs) const;
    };

    struct deferred_terminal_tag{};

    class IManagedBuffer
    {
    public:
        virtual ~IManagedBuffer() = default;
        /**
         * Mark a region of memory with the given parameters and region name.
         * 
         * Note: If a region already exists with this name, it will not be overwritten and this method will have no effect.
         * @param offset_bytes Number of bytes from the beginning of the buffer data that this region begins.
         * @param size_bytes Number of bytes comprising the size of the region.
         * @param name String representing the name of the region.
         * @return Structure describing the resultant region.
         */
        virtual void region(std::size_t offset_bytes, std::size_t size_bytes, std::string name) = 0;
        /**
         * Retrieve information about a region with the given name.
         * 
         * @param name Name of the region to retrieve information about.
         * @return Information about the region of the given name.
         */
        virtual const ManagedBufferRegion& get(const std::string& name) const = 0;
        /**
         * Erase the region with the given name if there is one. If not, nothing happens.
         * @param region_name Name of the region to erase.
         */
        virtual void erase(const std::string& region_name) = 0;
        /**
         * Perform a defragmentation step, reordering regions such that region boundaries are contiguous in memory.
         * 
         * For example, if there is a 2-byte gap between two regions in a managed buffer, the regions (and the underlying memory) are moved such that there is no longer a gap.
         * The ordering of regions is to be preserved. For example, if region 'A' began before any other region, this will hold true after defragmentation.
         * 
         * Note: This process is named after the Microsoft Windows utility: Disk Defragmenter
         * @return True if any regions were moved. Otherwise false.
         */
        virtual bool defragment() = 0;
        /**
         * Retrieve the number of bytes of the buffer data which belong to a region.
         * 
         * If all of the buffer data is regioned, then this will return a value equal to this->size().
         * @return Amount of buffer data regioned.
         */
        virtual std::size_t regions_usage() const = 0;
        /**
         * Query as to whether all of the buffer data belongs in regions.
         * This will always return false unless ALL bytes of the buffer data are in a region.
         * Note: This has no requirement of all bytes being in a single region. If all bytes are in their own regions, this will return true.
         * 
         * @return True if all data belongs in any region, false otherwise.
         */
        virtual bool regions_full() const = 0;
    };

    template<tz::gl::BufferType Type>
    class ManagedNonterminalBuffer : public IManagedBuffer, public Buffer<Type>
    {
    public:
        ManagedNonterminalBuffer(tz::gl::Object& holder);
        virtual void region(std::size_t offset_bytes, std::size_t size_bytes, std::string name) override;
        virtual const ManagedBufferRegion& get(const std::string& name) const override;
        virtual void erase(const std::string& region_name) override;
        virtual bool defragment() override;
        virtual std::size_t regions_usage() const override;
        virtual bool regions_full() const override;

        const ManagedBufferRegion& operator[](const std::string& name) const;
        const std::string& operator[](std::size_t idx) const;
    private:
        using MapType = tz::mem::DeMap<std::string, ManagedBufferRegion>;
        void verify_nonterminal() const;
        bool relocate_region(std::string region_name, std::size_t byte_index);
        MapType::iterator find_region_iter(const std::string& name);
        MapType::const_iterator find_region_iter(const std::string& name) const;

        tz::gl::Object& holder;
        MapType regions;
    };

    template<tz::gl::BufferType Type>
    class ManagedTerminalBuffer : public IManagedBuffer, public Buffer<Type>
    {
    public:
        ManagedTerminalBuffer(tz::gl::Object& holder, std::size_t size_bytes);
        ManagedTerminalBuffer(tz::gl::Object& holder, deferred_terminal_tag);
        virtual void region(std::size_t offset_bytes, std::size_t size_bytes, std::string name) override;
        virtual const ManagedBufferRegion& get(const std::string& name) const override;
        virtual void erase(const std::string& region_name) override;
        virtual bool defragment() override;
        virtual std::size_t regions_usage() const override;
        virtual bool regions_full() const override;

        virtual tz::mem::Block map(MappingPurpose purpose = MappingPurpose::ReadWrite) override;
        virtual void unmap() override;
        const ManagedBufferRegion& operator[](const std::string& name) const;
        const std::string& operator[](std::size_t idx) const;
    private:
        using MapType = tz::mem::DeMap<std::string, ManagedBufferRegion>;
        void verify_mapped() const;
        bool relocate_region(std::string region_name, std::size_t byte_index);
        MapType::iterator find_region_iter(const std::string& name);
        MapType::const_iterator find_region_iter(const std::string& name) const;

        tz::gl::Object& holder;
        MapType regions;
        std::optional<tz::mem::Block> mapped_block;
    };

    using ManagedVBO = ManagedNonterminalBuffer<tz::gl::BufferType::Array>;
    using ManagedIBO = ManagedNonterminalBuffer<tz::gl::BufferType::Index>;
    using ManagedUBO = ManagedNonterminalBuffer<tz::gl::BufferType::UniformStorage>;
    using ManagedSSBO = ManagedNonterminalBuffer<tz::gl::BufferType::ShaderStorage>;
    
    using ManagedTVBO = ManagedTerminalBuffer<tz::gl::BufferType::Array>;
    using ManagedTIBO = ManagedTerminalBuffer<tz::gl::BufferType::Index>;
    using ManagedTUBO = ManagedTerminalBuffer<tz::gl::BufferType::UniformStorage>;
    using ManagedTSSBO = ManagedTerminalBuffer<tz::gl::BufferType::ShaderStorage>;

}

#include "gl/managed_buffer.inl"
#endif // TOPAZ_GL_MANAGED_BUFFER_HPP