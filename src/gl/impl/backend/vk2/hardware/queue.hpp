#ifndef TOPAZ_GL_IMPL_BACKEND_VK2_HARDWARE_QUEUE_HPP
#define TOPAZ_GL_IMPL_BACKEND_VK2_HARDWARE_QUEUE_HPP
#if TZ_VULKAN
#include "core/containers/basic_list.hpp"
#include "vulkan/vulkan.h"
#include <cstdint>

namespace tz::gl::vk2
{
	class LogicalDevice;
	class CommandBuffer;
	class BinarySemaphore;

	/**
	 * @ingroup tz_gl_vk_graphics_pipeline
	 * Specifies a certain stage (point of execution) during the invocation of a graphics pipeline.
	 */
	enum class PipelineStage
	{
		/// - Very first stage when acting as a 'source' stage of a transition.
		Top = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		/// - Very first stage when acting as a 'destination' stage of a transition.
		Bottom = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		/// - Very first stage. Early enough to cover every single stage.
		AllCommands = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		/// - Pseudo-stage which takes place before any graphical stages (Just before DrawIndirect)
		AllGraphics = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
		/// - Stage of the pipeline where indirect draw commands are consumed.
		DrawIndirect = VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
		/// -  Stage of the pipeline where vertex and index buffers are consumed.
		VertexIndexInput = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
		/// - Stage of the pipeline where vertex shader is invoked.
		VertexShader = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
		/// - Stage of the pipeline where the tessellation control shader is invoked.
		TessellationControlShader = VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT,
		/// - Stage of the pipeline where the tessellation evaluation shader is invoked.
		TessellationEvaluationShader = VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT,
		/// - Stage of the pipeline where the geometry shader is invoked.
		GeometryShader = VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT,
		/// - Stage of the pipeline where the fragment shader is invoked.
		FragmentShader = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		/// - Stage of the pipeline directly *before* the fragment shader is invoked.
		EarlyFragmentTests = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
		/// - Stage of the pipeline directly *after* the fragment shader is invoked.
		LateFragmentTests = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
		/// - Stage of the pipeline where the final colour values are output from the pipeline.
		ColourAttachmentOutput = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		ComputeShader = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		/// - Stage of the pipeline where transfer commands are invoked. Example: Buffer copy, blit image, clears etc
		TransferCommands = VK_PIPELINE_STAGE_TRANSFER_BIT,
	};


	namespace hardware
	{
		struct QueueInfo
		{
			const LogicalDevice* dev;
			std::uint32_t queue_family_idx;
			std::uint32_t queue_idx;
		};

		class Queue
		{
		public:
			struct SubmitInfo
			{
				struct WaitInfo
				{
					const BinarySemaphore* wait_semaphore;
					PipelineStage wait_stage;
				};

				tz::BasicList<const CommandBuffer*> command_buffers;
				tz::BasicList<WaitInfo> waits;
				tz::BasicList<const BinarySemaphore*> signal_semaphores;
			};
			Queue(QueueInfo info);

			const QueueInfo& get_info() const;
			const LogicalDevice& get_device() const;
			void submit(SubmitInfo submit_info);

			using NativeType = VkQueue;
			NativeType native() const;
		private:
			VkQueue queue;
			QueueInfo info;
		};
	}
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_IMPL_BACKEND_VK2_HARDWARE_QUEUE_HPP
