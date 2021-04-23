#ifndef TOPAZ_GL_VK_ATTRIBUTE_BINDING_DESCRIPTION_HPP
#define TOPAZ_GL_VK_ATTRIBUTE_BINDING_DESCRIPTION_HPP
#if TZ_VULKAN
#include "vulkan/vulkan.h"
#include "core/assert.hpp"
#include <cstdint>

namespace tz::gl::vk
{

    class VertexAttributeDescription
    {
    public:
        constexpr VertexAttributeDescription(std::uint32_t binding, std::uint32_t location, VkFormat vertex_format, std::uint32_t offset):
        desc()
        {
            this->desc.binding = binding;
            this->desc.location = location;
            this->desc.format = vertex_format;
            this->desc.offset = offset;
        }

        VkVertexInputAttributeDescription native() const;
    private:
        VkVertexInputAttributeDescription desc;
    };

    using VertexAttributeDescriptions = std::initializer_list<VertexAttributeDescription>;
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_VK_VERTEX_BINDING_DESCRIPTION_HPP