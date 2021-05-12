#ifndef TOPAZ_GL_IMPL_VK_RENDERER_HPP
#define TOPAZ_GL_IMPL_VK_RENDERER_HPP
#include "gl/api/renderer.hpp"

#include "gl/vk/pipeline/input_assembly.hpp"
#include "gl/vk/pipeline/vertex_input.hpp"
#include "gl/vk/pipeline/rasteriser_state.hpp"

#include "gl/vk/swapchain.hpp"

namespace tz::gl
{
    class RendererBuilderVulkan : public IRendererBuilder
    {
    public:
        RendererBuilderVulkan() = default;
        virtual void set_element_format(RendererElementFormat element_format) final;
        virtual RendererElementFormat get_element_format() const final;
        virtual void set_culling_strategy(RendererCullingStrategy culling_strategy) final;
        virtual RendererCullingStrategy get_culling_strategy() const final;
        
        vk::pipeline::VertexInputState vk_get_vertex_input() const;
        vk::pipeline::RasteriserState vk_get_rasteriser_state() const;
    private:
        std::optional<RendererElementFormat> format = std::nullopt;
        RendererCullingStrategy culling_strategy = RendererCullingStrategy::NoCulling;
    };

    struct RendererBuilderDeviceInfoVulkan
    {
        vk::pipeline::PrimitiveTopology primitive_type;
        const vk::Swapchain* device_swapchain;
    };

    class RendererVulkan : public IRenderer
    {
    public:
        RendererVulkan(RendererBuilderVulkan builder, RendererBuilderDeviceInfoVulkan);
    private:
        vk::pipeline::InputAssembly input_assembly;
        vk::pipeline::VertexInputState vertex_input;
        vk::pipeline::RasteriserState rasteriser_state;
        const vk::Swapchain* swapchain;
    };
}

#endif // TOPAZ_GL_IMPL_VK_RENDERER_HPP