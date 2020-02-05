#include "gl/managed_buffer.hpp"
#include "core/debug/assert.hpp"

namespace tz::gl
{
    bool ManagedBufferRegion::operator<(const ManagedBufferRegion& rhs) const
    {
        return this->offset < rhs.offset;
    }
}