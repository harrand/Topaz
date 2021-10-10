#if TZ_VULKAN
#include "gl/impl/backend/vk/pipeline/graphics_pipeline.hpp"
#include "core/assert.hpp"
#include <algorithm>

namespace tz::gl::vk
{
	GraphicsPipeline::GraphicsPipeline
	(
		tz::BasicList<pipeline::ShaderStage> shader_stages,
		const LogicalDevice& device,
		pipeline::VertexInputState vertex_input,
		pipeline::InputAssembly input_assembly,
		pipeline::ViewportState viewport_state,
		pipeline::RasteriserState rasterisation_state,
		pipeline::MultisampleState multisample_state,
		pipeline::ColourBlendState colour_blend_state,
		pipeline::DynamicState dynamic_state,
		const pipeline::Layout& layout,
		const RenderPass& render_pass
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
		auto dynamic_state_native = dynamic_state.native();

		VkPipelineDepthStencilStateCreateInfo depth_stencil{};
		depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depth_stencil.depthTestEnable = VK_TRUE;
		depth_stencil.depthWriteEnable = VK_TRUE;

		depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;

		depth_stencil.depthBoundsTestEnable = VK_FALSE;
		depth_stencil.minDepthBounds = 0.0f; // Optional
		depth_stencil.maxDepthBounds = 1.0f; // Optional

		depth_stencil.stencilTestEnable = VK_FALSE;
		depth_stencil.front = {}; // Optional
		depth_stencil.back = {}; // Optional

		create.pVertexInputState = &vertex_input_native;
		create.pInputAssemblyState = &input_assembly_native;
		create.pViewportState = &viewport_state_native;
		create.pRasterizationState = &rasterisation_state_native;
		create.pMultisampleState = &multisample_state_native;
		create.pDepthStencilState = &depth_stencil;
		create.pColorBlendState = &colour_blend_state_native;
		create.pDynamicState = &dynamic_state_native;

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

	void GraphicsPipeline::bind(const CommandBuffer& command_buffer) const
	{
		vkCmdBindPipeline(command_buffer.native(), VK_PIPELINE_BIND_POINT_GRAPHICS, this->graphics_pipeline);
	}
}

#endif // TZ_VULKAN