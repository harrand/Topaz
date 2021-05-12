#ifndef TOPAZ_GL_IMPL_VK_RENDERER_HPP
#define TOPAZ_GL_IMPL_VK_RENDERER_HPP
#include "gl/api/renderer.hpp"
#include "gl/vk/pipeline/input_assembly.hpp"
#include "gl/vk/pipeline/vertex_input.hpp"

namespace tz::gl
{
    class RendererBuilderVulkan : public IRendererBuilder
    {
    public:
        RendererBuilderVulkan() = default;
        virtual void set_element_format(RendererElementFormat element_format) final;
        virtual RendererElementFormat get_element_format() const final;
        vk::pipeline::VertexInputState vk_get_vertex_input() const;
    private:
        std::optional<RendererElementFormat> format;
    };

    struct RendererBuilderDeviceInfoVulkan
    {
        vk::pipeline::PrimitiveTopology primitive_type;
    };

    class RendererVulkan : public IRenderer
    {
    public:
        RendererVulkan(RendererBuilderVulkan builder, RendererBuilderDeviceInfoVulkan);
    private:
        vk::pipeline::InputAssembly input_assembly;
        vk::pipeline::VertexInputState vertex_input;
    };
}

#endif // TOPAZ_GL_IMPL_VK_RENDERER_HPP