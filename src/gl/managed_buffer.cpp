#include "gl/managed_buffer.hpp"
#include "core/debug/assert.hpp"

namespace tz::gl
{
    bool ManagedNonterminalBufferRegion::operator<(const ManagedNonterminalBufferRegion& rhs) const
    {
        return this->offset < rhs.offset;
    }
    
    bool ManagedTerminalBufferRegion::operator<(const ManagedTerminalBufferRegion& rhs) const
    {
        topaz_assert(this->mapping_begin == rhs.mapping_begin, "tz::gl::ManagedTerminalBufferRegion::operator<(const ManagedTerminalBufferRegion&): Regions did not share the same common mapping beginning. This is illegal.");
        auto dist = [this](void* addr){return tz::mem::byte_distance(mapping_begin, addr);};
        return dist(this->block.begin) < dist(rhs.block.begin);
    }
}