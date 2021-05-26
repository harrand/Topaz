#include "gl/resource.hpp"

namespace tz::gl
{
    BufferResource::BufferResource(BufferResourceData data):
    data(data){}

    std::span<const std::byte> BufferResource::get_resource_data() const
    {
        return {this->data.data.begin(), this->data.data.end()};
    }
}