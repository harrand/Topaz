#if TZ_VULKAN
#include "gl/vk/pipeline/graphics_pipeline.hpp"
#include "core/assert.hpp"
#include <algorithm>

namespace tz::gl::vk
{
    GraphicsPipeline::GraphicsPipeline
    (
        std::initializer_list<pipeline::ShaderStage> shader_stages,
        const LogicalDevice& device,
        pipeline::VertexInputState vertex_input,
        pipeline::InputAssembly input_assembly,
        pipeline::ViewportState viewport_state,
        pipeline::RasteriserState rasterisation_state,
        pipeline::MultisampleState multisample_state,
        pipeline::ColourBlendState colour_blend_state,
        const pipeline::Layout& layout,
        const pipeline::RenderPass& render_pass
    ):
    shaders(shader_stages),
    device(&device),
    graphics_pipeline(VK_NULL_HANDLE)
    {
        VkGraphicsPipelineCreateInfo create{};
        create.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        create.stageCount = this->shaders.length();
        std::vector<VkPipelineShaderStageCreateInfo> stage_infos;
        for(const auto& shader_stage : this->shaders)
        {
            stage_infos.push_back(shader_stage.native());
        }
        create.pStages = stage_infos.data();

        // This is absurd
        auto vertex_input_native = vertex_input.native();
        auto input_assembly_native = input_assembly.native();
        auto viewport_state_native = viewport_state.native();
        auto rasterisation_state_native = rasterisation_state.native();
        auto multisample_state_native = multisample_state.native();
        auto colour_blend_state_native = colour_blend_state.native();

        create.pVertexInputState = &vertex_input_native;
        create.pInputAssemblyState = &input_assembly_native;
        create.pViewportState = &viewport_state_native;
        create.pRasterizationState = &rasterisation_state_native;
        create.pMultisampleState = &multisample_state_native;
        create.pDepthStencilState = nullptr;
        create.pColorBlendState = &colour_blend_state_native;
        create.pDynamicState = nullptr;

        create.layout = layout.native();
        create.renderPass = render_pass.native();
        create.basePipelineHandle = VK_NULL_HANDLE;
        create.basePipelineIndex= -1;

        auto res = vkCreateGraphicsPipelines(this->device->native(), VK_NULL_HANDLE, 1, &create, nullptr, &this->graphics_pipeline);
        tz_assert(res == VK_SUCCESS, "tz::gl::vk::GraphicsPipeline::GraphicsPipeline(...): Failed to create graphics pipeline");
    }

    GraphicsPipeline::GraphicsPipeline(GraphicsPipeline&& move):
    shaders(),
    device(nullptr),
    graphics_pipeline(VK_NULL_HANDLE)
    {
        *this = std::move(move);
    }

    GraphicsPipeline::~GraphicsPipeline()
    {
        if(this->graphics_pipeline != VK_NULL_HANDLE)
        {
            vkDestroyPipeline(this->device->native(), this->graphics_pipeline, nullptr);
            this->graphics_pipeline = VK_NULL_HANDLE;
        }
    }

    GraphicsPipeline& GraphicsPipeline::operator=(GraphicsPipeline&& rhs)
    {
        std::swap(this->shaders, rhs.shaders);
        std::swap(this->device, rhs.device);
        std::swap(this->graphics_pipeline, rhs.graphics_pipeline);
        return *this;
    }
}

#endif // TZ_VULKAN