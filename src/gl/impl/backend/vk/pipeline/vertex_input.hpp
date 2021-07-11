#ifndef TOPAZ_GL_VK_PIPELINE_VERTEX_INPUT_HPP
#define TOPAZ_GL_VK_PIPELINE_VERTEX_INPUT_HPP
#if TZ_VULKAN
#include "gl/impl/backend/vk/attribute_binding_description.hpp"
#include "gl/impl/backend/vk/vertex_binding_description.hpp"
#include <vector>

namespace tz::gl::vk::pipeline
{
    class VertexInputState
    {
    public:
        VertexInputState();
        VertexInputState(VertexBindingDescriptions binding_descriptions, VertexAttributeDescriptions attribute_descriptions);
        VkPipelineVertexInputStateCreateInfo native() const;
    private:
        VkPipelineVertexInputStateCreateInfo create;
        std::vector<VkVertexInputBindingDescription> binding_description_natives;
        std::vector<VkVertexInputAttributeDescription> attribute_description_natives;
    };
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_VK_PIPELINE_VERTEX_INPUT_HPP