#ifndef TOPAZ_GL_VK_PIPELINE_GRAPHICS_PIPELINE_HPP
#define TOPAZ_GL_VK_PIPELINE_GRAPHICS_PIPELINE_HPP
#if TZ_VULKAN

#include "gl/vk/impl/pipeline/shader_stage.hpp"
#include "core/containers/basic_list.hpp"

#include "gl/vk/impl/pipeline/vertex_input.hpp"
#include "gl/vk/impl/pipeline/input_assembly.hpp"
#include "gl/vk/impl/pipeline/viewport.hpp"
#include "gl/vk/impl/pipeline/rasteriser_state.hpp"
#include "gl/vk/impl/pipeline/multisample_state.hpp"
#include "gl/vk/impl/pipeline/colour_blend_state.hpp"
#include "gl/vk/impl/pipeline/layout.hpp"
//#include "gl/vk/impl/pipeline/render_pass.hpp"
#include "gl/vk/render_pass.hpp"

namespace tz::gl::vk
{
    using ShaderPipeline = tz::BasicList<pipeline::ShaderStage>;
    
    class GraphicsPipeline
    {
    public:
        GraphicsPipeline
        (
            std::initializer_list<pipeline::ShaderStage> shader_stages,
            const LogicalDevice& device,
            pipeline::VertexInputState vertex_input,
            pipeline::InputAssembly input_assembly,
            pipeline::ViewportState viewport_state,
            pipeline::RasteriserState rasterisation_state,
            pipeline::MultisampleState multisample_state,
            /*TODO: DepthStencilState*/
            pipeline::ColourBlendState colour_blend_state,
            /*TODO: DynamicState*/
            const pipeline::Layout& layout,
            const RenderPass& render_pass
        );
        GraphicsPipeline(const GraphicsPipeline& copy) = delete;
        GraphicsPipeline(GraphicsPipeline&& move);
        ~GraphicsPipeline();

        GraphicsPipeline& operator=(const GraphicsPipeline& rhs) = delete;
        GraphicsPipeline& operator=(GraphicsPipeline&& rhs);
    private:
        ShaderPipeline shaders;
        const LogicalDevice* device;
        VkPipeline graphics_pipeline;
    };
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_VK_PIPELINE_GRAPHICS_PIPELINE_HPP