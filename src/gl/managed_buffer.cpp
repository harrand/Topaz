#include "gl/managed_buffer.hpp"
#include "core/debug/assert.hpp"

namespace tz::gl
{
    bool ManagedBufferRegion::operator<(const ManagedBufferRegion& rhs) const
    {
        topaz_assert(this->mapping_begin == rhs.mapping_begin, "tz::gl::ManagedBufferRegion::operator<(const ManagedBufferRegion&): Regions did not share the same common mapping beginning. This is illegal.");
        auto dist = [this](void* addr){return tz::mem::byte_distance(mapping_begin, addr);};
        return dist(this->block.begin) < dist(rhs.block.begin);
    }
}