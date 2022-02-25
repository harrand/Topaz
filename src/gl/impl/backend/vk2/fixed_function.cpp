#if TZ_VULKAN
#include "gl/impl/backend/vk2/fixed_function.hpp"

namespace tz::gl::vk2
{
	ViewportState::NativeType ViewportState::native() const
	{
		return
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.viewportCount = static_cast<std::uint32_t>(this->viewports.length()),
			.pViewports = this->viewports.data(),
			.scissorCount = static_cast<std::uint32_t>(this->scissors.length()),
			.pScissors = this->scissors.data()
		};
	}

	ViewportState create_basic_viewport(tz::Vec2 dimensions)
	{
		ViewportState state;
		state.viewports.add({});
		VkViewport& viewport = state.viewports.back();

		// OGL-like coordinate system. Note that ogl [mindepth, maxdepth] is [-1, 1] but we are forced to have it [0, 1] without extensions. This means we have no safe way of matching behaviour. For now the shaders have to do this manually :(
		viewport.x = 0.0f;
		viewport.y = dimensions[1];
		viewport.width = dimensions[0];
		viewport.height = -dimensions[1];
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		state.scissors.add({});
		VkRect2D& scissor = state.scissors.back();
		scissor.offset = {0, 0};
		scissor.extent = {static_cast<std::uint32_t>(dimensions[0]), static_cast<std::uint32_t>(dimensions[1])};

		return state;
	}

	RasteriserState::NativeType RasteriserState::native() const
	{
		return
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO, 
			.pNext = nullptr,
			.flags = 0,
			.depthClampEnable = this->depth_clamp ? VK_TRUE : VK_FALSE,
			.rasterizerDiscardEnable = VK_FALSE,
			.polygonMode = static_cast<VkPolygonMode>(this->polygon_mode),
			.cullMode = static_cast<VkCullModeFlags>(this->cull_mode),
			.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
			.depthBiasEnable = VK_FALSE,
			.depthBiasConstantFactor = 0.0f,
			.depthBiasClamp = 0.0f,
			.depthBiasSlopeFactor = 0.0f,
			.lineWidth = 1.0f
		};
	}

	MultisampleState::NativeType MultisampleState::native() const
	{
		return
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
			.sampleShadingEnable = VK_FALSE,
			.minSampleShading = 1.0f,
			.pSampleMask = nullptr,
			.alphaToCoverageEnable = VK_FALSE,
			.alphaToOneEnable = VK_FALSE
		};
	}

	DepthStencilState::NativeType DepthStencilState::native() const
	{
		return
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.depthTestEnable = this->depth_testing ? VK_TRUE : VK_FALSE,
			.depthWriteEnable = this->depth_writes ? VK_TRUE : VK_FALSE,
			.depthCompareOp = static_cast<VkCompareOp>(this->depth_compare_operation),
			.depthBoundsTestEnable = this->depth_bounds_testing ? VK_TRUE : VK_FALSE,
			.stencilTestEnable = this->stencil_testing ? VK_TRUE : VK_FALSE,
			.front = this->front,
			.back = this->back,
			.minDepthBounds = this->min_depth_bounds,
			.maxDepthBounds = this->max_depth_bounds
		};
	}

	ColourBlendState::AttachmentState ColourBlendState::no_blending()
	{
		return
		{
			.blendEnable = VK_FALSE,
			.srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
			.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
			.colorBlendOp = VK_BLEND_OP_ADD,
			.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
			.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
			.alphaBlendOp = VK_BLEND_OP_ADD,
			.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
		};
	}

	ColourBlendState::NativeType ColourBlendState::native() const
	{
		ColourBlendState::NativeType native
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.logicOpEnable = this->logical_operator.has_value() ? VK_TRUE : VK_FALSE,
			.logicOp = this->logical_operator.value_or(VkLogicOp{}),
			.attachmentCount = static_cast<std::uint32_t>(this->attachment_states.length()),
			.pAttachments = this->attachment_states.data(),
		};
		for(std::size_t i = 0; i < sizeof(native.blendConstants) / sizeof(native.blendConstants[0]); i++)
		{
			native.blendConstants[i] = 0.0f;
		}
		return native;
	}

	DynamicState::NativeType DynamicState::native() const
	{
		return
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.dynamicStateCount = 0,
			.pDynamicStates = nullptr
		};
	}
}

#endif // TZ_VULKAN
