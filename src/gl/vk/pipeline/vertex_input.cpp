#if TZ_VULKAN
#include "gl/vk/pipeline/vertex_input.hpp"

namespace tz::gl::vk::pipeline
{
    VertexInputState::VertexInputState(): VertexInputState(VertexBindingDescriptions{}, VertexAttributeDescriptions{}){}

    VertexInputState::VertexInputState(VertexBindingDescriptions binding_descriptions, VertexAttributeDescriptions attribute_descriptions):
    create(),
    binding_description_natives(),
    attribute_description_natives()
    {
        for(const auto& binding_desc : binding_descriptions)
        {
            this->binding_description_natives.push_back(binding_desc.native());
        }

        for(const auto& attribute_desc : attribute_descriptions)
        {
            this->attribute_description_natives.push_back(attribute_desc.native());
        }

        this->create.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        this->create.vertexBindingDescriptionCount = this->binding_description_natives.size();
        this->create.pVertexBindingDescriptions = this->binding_description_natives.data();
        this->create.vertexAttributeDescriptionCount = this->attribute_description_natives.size();
        this->create.pVertexAttributeDescriptions = this->attribute_description_natives.data();
    }


    VkPipelineVertexInputStateCreateInfo VertexInputState::native() const
    {
        return this->create;
    }
}

#endif // TZ_VULKAN