#include "gl/object.hpp"
#include "core/debug/assert.hpp"
#include <cstring>

namespace tz::gl
{
    template<tz::gl::BufferType Type>
    ManagedTerminalBuffer<Type>::ManagedTerminalBuffer(tz::gl::Object& holder, std::size_t size_bytes): IManagedTerminalBuffer(), Buffer<Type>(), holder(holder), regions(), mapped_block(std::nullopt)
    {
        this->terminal_resize(size_bytes);
    }

    template<tz::gl::BufferType Type>
    ManagedTerminalBuffer<Type>::ManagedTerminalBuffer(tz::gl::Object& holder, deferred_terminal_tag): IManagedTerminalBuffer(), Buffer<Type>(), holder(holder), regions(), mapped_block(std::nullopt){}

    template<tz::gl::BufferType Type>
    ManagedTerminalBufferRegion ManagedTerminalBuffer<Type>::region(std::size_t offset_bytes, std::size_t size_bytes, std::string name)
    {
        this->verify_mapped();
        char* mapped_begin = reinterpret_cast<char*>(this->mapped_block.value().begin);
        char* offsetted_begin = mapped_begin + static_cast<std::ptrdiff_t>(offset_bytes);
        auto emplacement_pair = regions.emplace(name, ManagedTerminalBufferRegion{mapped_begin, name, {offsetted_begin, size_bytes}});
        return *(emplacement_pair.second);
    }

    template<tz::gl::BufferType Type>
    void ManagedTerminalBuffer<Type>::erase(const std::string& region_name)
    {
        this->regions.erase_key(region_name);
    }

    template<tz::gl::BufferType Type>
    bool ManagedTerminalBuffer<Type>::defragment()
    {
        bool movement = false;
        this->verify_mapped();
        // early out if we have full usage already
        if(this->regions_full())
            return false;
        // named after disk defragmentation
        std::size_t byte_count = 0;
        for(const auto& [region_name, region] : this->regions)
        {
            topaz_assert(byte_count < this->mapped_block.value().size(), "tz::gl::ManagedTerminalBuffer<Type>::defragment(): Internal error: Byte count exceeded mapping size!");
            std::size_t const region_size = region->block.size();
            movement |= this->relocate_region(*region_name, byte_count);
            byte_count += region_size;
        }
        return movement;
    }

    template<tz::gl::BufferType Type>
    std::size_t ManagedTerminalBuffer<Type>::regions_usage() const
    {
        std::size_t total_size = 0;
        for(const auto& [region_name, region] : this->regions)
        {  
            total_size += region->block.size();
        }
        return total_size;
    }

    template<tz::gl::BufferType Type>
    bool ManagedTerminalBuffer<Type>::regions_full() const
    {
        return this->size() == this->regions_usage();
    }

    template<tz::gl::BufferType Type>
    tz::mem::Block ManagedTerminalBuffer<Type>::map(MappingPurpose purpose)
    {
        this->mapped_block = {Buffer<Type>::map(purpose)};
        return this->mapped_block.value();
    }

    template<tz::gl::BufferType Type>
    void ManagedTerminalBuffer<Type>::unmap()
    {
        Buffer<Type>::unmap();
        this->regions.clear();
    }

    template<tz::gl::BufferType Type>
    const ManagedTerminalBufferRegion& ManagedTerminalBuffer<Type>::operator[](const std::string& name) const
    {
        this->verify_mapped();
        auto find_result = this->find_region_iter(name);
        topaz_assert(find_result != this->regions.cend(), "tz::gl::ManagedTerminalBuffer<Type>::operator[", name, "]: No such region exists with the name \"", name, "\"");
        return *((*find_result).second);
    }

    template<tz::gl::BufferType Type>
    const std::string& ManagedTerminalBuffer<Type>::operator[](std::size_t idx) const
    {
        auto iter = this->regions.begin() + idx;
        return *((*iter).first);
    }

    template<tz::gl::BufferType Type>
    void ManagedTerminalBuffer<Type>::verify_mapped() const
    {
        topaz_assert(this->mapped_block.has_value() && this->is_mapped(), "tz::gl::ManagedTerminalBuffer<Type>::verify_mapped(): Verification failed due to buffer not being terminal.");
        topaz_assert(this->is_terminal(), "tz::gl::ManagedTerminalBuffer<Type>::verify_mapped(): Verification failed due to buffer not being terminal.");
    }

    template<tz::gl::BufferType Type>
    bool ManagedTerminalBuffer<Type>::relocate_region(std::string region_name, std::size_t byte_index)
    {
        this->verify_mapped();
        // pop the entry out from the demap, amend the mem block information, and then insert the entry at the new block position.
        void* mapping_begin = this->mapped_block.value().begin;
        topaz_assert(this->regions.contains_key(region_name), "tz::gl::ManagedTerminalBuffer<Type>::relocate_region(", region_name, ", ", byte_index, "): No such region named \"", region_name, "\"");
        ManagedTerminalBufferRegion region = this->regions.get_value(region_name);

        auto region_before = region.block;
        const std::size_t region_size_bytes = region_before.size();
        std::size_t byte_index_before = tz::mem::byte_distance(mapping_begin, region_before.begin);
        // Is it already at this position? Nice -- early out.
        if(byte_index == byte_index_before)
            return false;
        
        this->regions.erase_key(region_name);

        // Note: this will not invalidate mapped terminal pools, so we should be very careful to ensure moved elements move the underlying type to its new memory location too.
        // we're not gonna do any checking here. if something important is going to be in our new memory block -- tough shit
        char* destination_address = reinterpret_cast<char*>(mapping_begin) + byte_index;
        char* source_address = reinterpret_cast<char*>(mapping_begin) + byte_index_before;
        // do the copy.
        std::memcpy(destination_address, source_address, region.block.size());
        // now edit our region to point to this new area.
        region.block = {destination_address, region_size_bytes};
        this->regions.set_value(region_name, region);
        return true;
    }

    template<tz::gl::BufferType Type>
    const char* ManagedTerminalBuffer<Type>::region_within(std::size_t byte_index) const
    {
         
    }

    template<tz::gl::BufferType Type>
    typename ManagedTerminalBuffer<Type>::MapType::iterator ManagedTerminalBuffer<Type>::find_region_iter(const std::string& name)
    {
        for(auto iter = this->regions.begin(); iter != this->regions.end(); iter++)
        {
            if(*(*iter).first == name)
                return iter;
        }
        return this->regions.end();
    }

    template<tz::gl::BufferType Type>
    typename ManagedTerminalBuffer<Type>::MapType::const_iterator ManagedTerminalBuffer<Type>::find_region_iter(const std::string& name) const
    {
        for(auto iter = this->regions.begin(); iter != this->regions.end(); ++iter)
        {
            if(*(*iter).first == name)
                return iter;
        }
        return this->regions.end();
    }

}