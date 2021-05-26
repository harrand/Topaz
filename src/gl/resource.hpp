#ifndef TOPAZ_GL_RESOURCE_HPP
#define TOPAZ_GL_RESOURCE_HPP
#include "core/containers/basic_list.hpp"
#include "gl/api/resource.hpp"

namespace tz::gl
{
    struct BufferResourceData
    {
        tz::BasicList<std::byte> data;
    };

    class BufferResource : public IResourceCopyable<BufferResource>
    {
    public:
        BufferResource(BufferResourceData data);
        virtual std::span<const std::byte> get_resource_data() const final;
    private:
        BufferResourceData data;
    };
}

#endif // TOPAZ_GL_RESOURCE_HPP