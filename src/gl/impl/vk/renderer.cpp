#if TZ_VULKAN
#include "gl/impl/vk/renderer.hpp"
#include "gl/impl/vk/device.hpp"

namespace tz::gl
{
    void RendererBuilderVulkan::set_input(const IRendererInput& input)
    {
        this->format = {input.get_format()};
    }

    RendererElementFormat RendererBuilderVulkan::get_input_format() const
    {
        tz_assert(this->format.has_value(), "RendererBuilder has not had element format set yet");
        return this->format.value();
    }

    void RendererBuilderVulkan::set_culling_strategy(RendererCullingStrategy culling_strategy)
    {
        this->culling_strategy = culling_strategy;
    }

    RendererCullingStrategy RendererBuilderVulkan::get_culling_strategy() const
    {
        return this->culling_strategy;
    }

    void RendererBuilderVulkan::set_render_pass(const RenderPass& render_pass)
    {
        this->render_pass = &render_pass;
    }

    const RenderPass& RendererBuilder::get_render_pass() const
    {
        tz_assert(this->render_pass != nullptr, "No render pass set");
        return *this->render_pass;
    }

    void RendererBuilderVulkan::set_shader(const Shader& shader)
    {
        this->shader = &shader;
    }

    const Shader& RendererBuilderVulkan::get_shader() const
    {
        tz_assert(this->shader != nullptr, "No shader yet");
        return *this->shader;
    }

    vk::pipeline::VertexInputState RendererBuilderVulkan::vk_get_vertex_input() const
    {
        RendererElementFormat fmt = this->get_input_format();
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

    vk::pipeline::RasteriserState RendererBuilderVulkan::vk_get_rasteriser_state() const
    {
        vk::pipeline::CullingStrategy vk_cull;
        switch(this->culling_strategy)
        {
            case RendererCullingStrategy::NoCulling:
                vk_cull = vk::pipeline::CullingStrategy::None;
            break;
            case RendererCullingStrategy::CullFrontFaces:
                vk_cull = vk::pipeline::CullingStrategy::Front;
            break;
            case RendererCullingStrategy::CullBackFaces:
                vk_cull = vk::pipeline::CullingStrategy::Back;
            break;
            case RendererCullingStrategy::CullEverything:
                vk_cull = vk::pipeline::CullingStrategy::Both;
            break;
        }
        return
        {
            false,
            false,
            vk::pipeline::PolygonMode::Fill,
            1.0f,
            vk_cull
        };
    }

    RendererVulkan::RendererVulkan(RendererBuilderVulkan builder, RendererBuilderDeviceInfoVulkan device_info):
    graphics_pipeline
    (
        {vk::pipeline::ShaderStage{builder.get_shader().vk_get_vertex_shader(), vk::pipeline::ShaderType::Vertex}, vk::pipeline::ShaderStage{builder.get_shader().vk_get_fragment_shader(), vk::pipeline::ShaderType::Fragment}},
        *device_info.device,
        builder.vk_get_vertex_input(),
        vk::pipeline::InputAssembly{device_info.primitive_type},
        vk::pipeline::ViewportState{*device_info.device_swapchain},
        builder.vk_get_rasteriser_state(),
        vk::pipeline::MultisampleState{},
        vk::pipeline::ColourBlendState{},
        vk::pipeline::DynamicState::None(),
        vk::pipeline::Layout{*device_info.device},
        builder.get_render_pass().vk_get_render_pass()
    )
    {
        
    }
}
#endif // TZ_VULKAN