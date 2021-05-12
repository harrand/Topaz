#if TZ_VULKAN
#include "gl/impl/vk/renderer.hpp"

namespace tz::gl
{
    void RendererBuilderVulkan::set_element_format(RendererElementFormat element_format)
    {
        this->format = {element_format};
    }

    RendererElementFormat RendererBuilderVulkan::get_element_format() const
    {
        tz_assert(this->format.has_value(), "RendererBuilder has not had element format set yet");
        return this->format.value();
    }

    vk::pipeline::VertexInputState RendererBuilderVulkan::vk_get_vertex_input() const
    {
        RendererElementFormat fmt = this->get_element_format();
        vk::VertexInputRate input_rate;
        switch(fmt.basis)
        {
            case RendererInputFrequency::PerVertexBasis:
                input_rate = vk::VertexInputRate::PerVertexBasis;
            break;
            case RendererInputFrequency::PerInstanceBasis:
                input_rate = vk::VertexInputRate::PerInstanceBasis;
            break;
            default:
                tz_error("Can't map RendererInputFrequency to vk::VertexInputRate");
            break;
        }
        vk::VertexBindingDescription binding{0, fmt.binding_size, input_rate};
        vk::VertexAttributeDescriptions attributes;
        for(std::size_t i = 0; i < fmt.binding_attributes.length(); i++)
        {
            RendererAttributeFormat attribute = fmt.binding_attributes[i];
            VkFormat vk_format;
            switch(attribute.type)
            {
                case RendererComponentType::Float32:
                    vk_format = VK_FORMAT_R32_SFLOAT;
                break;
                case RendererComponentType::Float32x2:
                    vk_format = VK_FORMAT_R32G32_SFLOAT;
                break;
                case RendererComponentType::Float32x3:
                    vk_format = VK_FORMAT_R32G32B32_SFLOAT;
                break;
                default:
                    tz_error("Unknown mapping from RendererComponentType to VkFormat");
                    vk_format = VK_FORMAT_UNDEFINED;
                break;
            }
            attributes.emplace_back(0, i, vk_format, attribute.element_attribute_offset);
        }
        return {vk::VertexBindingDescriptions{binding}, attributes};
    }

    RendererVulkan::RendererVulkan(RendererBuilderVulkan builder, RendererBuilderDeviceInfoVulkan device_info):
    input_assembly(device_info.primitive_type),
    vertex_input(builder.vk_get_vertex_input())
    {

    }
}
#endif // TZ_VULKAN