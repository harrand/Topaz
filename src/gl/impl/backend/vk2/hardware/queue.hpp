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
	class Swapchain;
	class Fence;

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

		/**
		 * @ingroup tz_gl_vk
		 * Represents a single hardware Queue.
		 */
		class Queue
		{
		public:
			/**
			 * Specifies information about a submission of GPU work to a Queue.
			 */
			struct SubmitInfo
			{
				struct WaitInfo
				{
					/// Semaphore which will be waited on.
					const BinarySemaphore* wait_semaphore;
					/// Information about when specifically the semaphore will be waited on.
					PipelineStage wait_stage;
				};

				/// List of command buffers to submit.
				tz::BasicList<const CommandBuffer*> command_buffers;
				/// List of wait semaphores. Command buffers for this submission batch will not begin execution until these semaphores are signalled. Note that when the wait is complete, each semaphore will be automatically set to unsignalled again.
				tz::BasicList<WaitInfo> waits;
				/// List of signal semaphores. Once the command buffers associated with this submission have completed execution, these semaphores are signalled.
				tz::BasicList<const BinarySemaphore*> signal_semaphores;
				/// Optional fence which is signalled once all command buffers have finished execution. If this is nullptr, there is no way to verify that the command buffers associated with this submission have completed.
				const Fence* execution_complete_fence;
			};

			/// Describes the result of a presentation request. See @ref Queue::present
			enum class PresentResult
			{
				/// - Presentation request succeeded without any issues.
				Success,
				/// - Presentation request succeeded, but swapchain no longer matches surface properly. It should be updated.
				Success_Suboptimal,
				/// - Presentation request failed because the swapchain is no longer comapatible with the surface. It should be updated.
				Fail_OutOfDate,
				/// - Presentation request failed because we did not have exclusive access to the swapchain.
				Fail_AccessDenied,
				/// - Presentation request failed because the surface is no longer available.
				Fail_SurfaceLost,
				/// - Presentation request failed, and there's nothing we can do about it.
				Fail_FatalError
			};

			/**
			 * Specifies information about a present request issued to a Queue.
			 */
			struct PresentInfo
			{
				/// List of wait semaphores. The present request will not be issued until all of these semaphores are signalled. Note that once they are signalled, they are automatically set to unsignalled again.
				tz::BasicList<const BinarySemaphore*> wait_semaphores;
				/// Pointer to a swapchain from which an image will be sourced. This must not be nullptr.
				const Swapchain* swapchain;
				/// Index to the swapchain's images which will be presented to the surface. The index must have value such that `swapchain_image_index < swapchain.get_image_views().size()`
				std::uint32_t swapchain_image_index;
			};

			Queue(QueueInfo info);

			const QueueInfo& get_info() const;
			const LogicalDevice& get_device() const;
			/**
			 * Submit the queue, including any associated command buffers and perform necessary synchronisation.
			 * See @ref SubmitInfo for more information.
			 */
			void submit(SubmitInfo submit_info);
			/**
			 * Queue an image for presentation.
			 * See @ref PresentInfo for details.
			 * @return Description on whether the presentation request was accepted or rejected.
			 */
			[[nodiscard]] PresentResult present(PresentInfo present_info);

			using NativeType = VkQueue;
			NativeType native() const;
		private:
			void execute_cpu_side_command_buffer(const CommandBuffer& command_buffer) const;

			VkQueue queue;
			QueueInfo info;
		};
	}
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_IMPL_BACKEND_VK2_HARDWARE_QUEUE_HPP
