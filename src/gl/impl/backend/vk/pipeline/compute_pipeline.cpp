#if TZ_VULKAN
#include "gl/impl/backend/vk/pipeline/compute_pipeline.hpp"
#include <utility>

namespace tz::gl::vk
{
	ComputePipeline::ComputePipeline
	(
		pipeline::ShaderStage compute_shader,
		const LogicalDevice& device,
		const pipeline::Layout& layout
	):
	compute_shader(std::move(compute_shader)),
	device(&device),
	compute_pipeline(VK_NULL_HANDLE)
	{
		VkComputePipelineCreateInfo create{};
		create.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		create.stage = this->compute_shader.value().native();
		create.layout = layout.native();
		create.basePipelineHandle = VK_NULL_HANDLE;
		create.basePipelineIndex = 0;

		auto res = vkCreateComputePipelines(this->device->native(), VK_NULL_HANDLE, 1, &create, nullptr, &this->compute_pipeline);
		tz_assert(res == VK_SUCCESS, "tz::gl::vk::ComputePipeline::ComputePipeline(...): Failed to create compute pipeline");
	}

	ComputePipeline::ComputePipeline(ComputePipeline&& move):
	compute_shader(std::nullopt),
	device(nullptr),
	compute_pipeline(VK_NULL_HANDLE)
	{
		*this = std::move(move);
	}

	ComputePipeline::~ComputePipeline()
	{
		if(this->compute_pipeline != VK_NULL_HANDLE)
		{
			vkDestroyPipeline(this->device->native(), this->compute_pipeline, nullptr);
			this->compute_pipeline = VK_NULL_HANDLE;
		}
	}

	ComputePipeline& ComputePipeline::operator=(ComputePipeline&& rhs)
	{
		std::swap(this->compute_shader, rhs.compute_shader);
		std::swap(this->device, rhs.device);
		std::swap(this->compute_pipeline, rhs.compute_pipeline);
		return *this;
	}

	void ComputePipeline::bind(const CommandBuffer& command_buffer) const
	{
		vkCmdBindPipeline(command_buffer.native(), VK_PIPELINE_BIND_POINT_COMPUTE, this->compute_pipeline);
	}
}

#endif // TZ_VULKAN