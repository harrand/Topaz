#ifndef TOPAZ_GL_VK_PIPELINE_GRAPHICS_PIPELINE_HPP
#define TOPAZ_GL_VK_PIPELINE_GRAPHICS_PIPELINE_HPP
#if TZ_VULKAN

#include "gl/impl/backend/vk/pipeline/shader_stage.hpp"
#include "core/containers/basic_list.hpp"

#include "gl/impl/backend/vk/pipeline/vertex_input.hpp"
#include "gl/impl/backend/vk/pipeline/input_assembly.hpp"
#include "gl/impl/backend/vk/pipeline/viewport.hpp"
#include "gl/impl/backend/vk/pipeline/rasteriser_state.hpp"
#include "gl/impl/backend/vk/pipeline/multisample_state.hpp"
#include "gl/impl/backend/vk/pipeline/colour_blend_state.hpp"
#include "gl/impl/backend/vk/pipeline/dynamic_state.hpp"
#include "gl/impl/backend/vk/pipeline/layout.hpp"
#include "gl/impl/backend/vk/render_pass.hpp"

namespace tz::gl::vk
{
    using ShaderPipeline = tz::BasicList<pipeline::ShaderStage>;
    
    class GraphicsPipeline
    {
    public:
        GraphicsPipeline
        (
            tz::BasicList<pipeline::ShaderStage> shader_stages,
            const LogicalDevice& device,
            pipeline::VertexInputState vertex_input,
            pipeline::InputAssembly input_assembly,
            pipeline::ViewportState viewport_state,
            pipeline::RasteriserState rasterisation_state,
            pipeline::MultisampleState multisample_state,
            /*TODO: DepthStencilState*/
            pipeline::ColourBlendState colour_blend_state,
            pipeline::DynamicState dynamic_state,
            const pipeline::Layout& layout,
            const RenderPass& render_pass
        );
        GraphicsPipeline(const GraphicsPipeline& copy) = delete;
        GraphicsPipeline(GraphicsPipeline&& move);
        ~GraphicsPipeline();

        GraphicsPipeline& operator=(const GraphicsPipeline& rhs) = delete;
        GraphicsPipeline& operator=(GraphicsPipeline&& rhs);

        void bind(const CommandBuffer& command_buffer) const;
    private:
        bool is_compute() const;
        ShaderPipeline shaders;
        const LogicalDevice* device;
        VkPipeline graphics_pipeline;
    };
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_VK_PIPELINE_GRAPHICS_PIPELINE_HPP