#if TZ_VULKAN
#include "gl/vk/pipeline/vertex_input.hpp"

namespace tz::gl::vk::pipeline
{
    VertexInputState::VertexInputState():
    create()
    {
        // TODO: Have it able to specify non-zero vertex data.
        this->create.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        this->create.vertexBindingDescriptionCount = 0;
        this->create.pVertexAttributeDescriptions = nullptr;
        this->create.vertexAttributeDescriptionCount = 0;
        this->create.pVertexBindingDescriptions = nullptr;
    }

    VkPipelineVertexInputStateCreateInfo VertexInputState::native() const
    {
        return this->create;
    }
}

#endif // TZ_VULKAN