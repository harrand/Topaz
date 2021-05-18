#if TZ_VULKAN
#include "gl/vk/vertex_binding_description.hpp"

namespace tz::gl::vk
{
    VkVertexInputBindingDescription VertexBindingDescription::native() const
    {
        return this->desc;
    }
}

#endif // TZ_VULKAN