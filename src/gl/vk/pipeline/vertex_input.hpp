#ifndef TOPAZ_GL_VK_PIPELINE_VERTEX_INPUT_HPP
#define TOPAZ_GL_VK_PIPELINE_VERTEX_INPUT_HPP
#if TZ_VULKAN
#include "vulkan/vulkan.h"

namespace tz::gl::vk::pipeline
{
    class VertexInputState
    {
    public:
        VertexInputState();
        VkPipelineVertexInputStateCreateInfo native() const;
    private:
        VkPipelineVertexInputStateCreateInfo create;
    };
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_VK_PIPELINE_VERTEX_INPUT_HPP