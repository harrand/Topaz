#if TZ_VULKAN
#include "gl/impl/backend/vk2/fixed_function.hpp"

namespace tz::gl::vk2
{
	InputAssembly::NativeType InputAssembly::native() const
	{
		return
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO, 
			.pNext = nullptr,
			.flags = 0,
			.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			.primitiveRestartEnable = VK_FALSE
		};
	}

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

		viewport.width = dimensions[0];
		viewport.height = dimensions[1];
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		state.scissors.add({});
		VkRect2D scissor = state.scissors.back();
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
			.frontFace = VK_FRONT_FACE_CLOCKWISE,
			.depthBiasEnable = VK_FALSE,
			.depthBiasConstantFactor = 0.0f,
			.depthBiasClamp = 0.0f,
			.depthBiasSlopeFactor = 0.0f,
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
}

#endif // TZ_VULKAN
