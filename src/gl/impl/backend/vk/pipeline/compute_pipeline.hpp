#ifndef TOPAZ_GL_IMPL_BACKEND_VK_PIPELINE_COMPUTE_PIPELINE_HPP
#define TOPAZ_GL_IMPL_BACKEND_VK_PIPELINE_COMPUTE_PIPELINE_HPP
#if TZ_VULKAN

#include "gl/impl/backend/vk/pipeline/shader_stage.hpp"

#include "gl/impl/backend/vk/pipeline/layout.hpp"
#include "gl/impl/backend/vk/command.hpp"

#include <optional>

namespace tz::gl::vk
{
	class ComputePipeline
	{
	public:
		ComputePipeline
		(
			pipeline::ShaderStage compute_shader,
			const LogicalDevice& device,
			const pipeline::Layout& layout
		);
		ComputePipeline(const ComputePipeline& copy) = delete;
		ComputePipeline(ComputePipeline&& move);
		~ComputePipeline();

		ComputePipeline& operator=(const ComputePipeline& rhs) = delete;
		ComputePipeline& operator=(ComputePipeline&& rhs);

		void bind(const CommandBuffer& command_buffer) const;
	private:
		std::optional<pipeline::ShaderStage> compute_shader;
		const LogicalDevice* device;
		VkPipeline compute_pipeline;
	};
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_IMPL_BACKEND_VK_PIPELINE_COMPUTE_PIPELINE_HPP