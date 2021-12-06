#ifndef TOPAZ_GL_IMPL_BACKEND_VK2_COMMAND_HPP
#define TOPAZ_GL_IMPL_BACKEND_VK2_COMMAND_HPP
#include "gl/impl/backend/vk2/render_pass.hpp"
#if TZ_VULKAN
#include "gl/impl/backend/vk/render_pass.hpp"
#include "gl/impl/backend/vk2/hardware/queue.hpp"
#include "gl/impl/backend/vk2/logical_device.hpp"
#include "gl/impl/backend/vk2/framebuffer.hpp"
#include "gl/impl/backend/vk2/graphics_pipeline.hpp"
#include "gl/impl/backend/vk2/buffer.hpp"

namespace tz::gl::vk2
{
	/**
	 * @ingroup tz_gl_vk_commands
	 * Contains all the possible commands which can be recorded within a @ref CommandBuffer.
	 */
	struct VulkanCommand
	{
		/**
		 * Record a non-indexed draw.
		 * See @ref CommandBufferRecording::draw for usage.
		 */
		struct Draw
		{
			/// Number of vertices to draw.
			std::uint32_t vertex_count;
			/// Number of instances to draw (Default 1).
			std::uint32_t instance_count = 1;
			/// Index of the first vertex to draw (Default 0).
			std::uint32_t first_vertex = 0;
			/// Instance ID of the first instance to draw (Default 0).
			std::uint32_t first_instance = 0;
		};

		/**
		 * Record a bind of a @ref GraphicsPipeline.
		 * See @ref CommandBufferRecording::bind_pipeline for usage.
		 */
		struct BindPipeline
		{
			/// Pipeline to be bound. Must not be null.
			const GraphicsPipeline* pipeline;
			/// Specify the pipeline context to bind to. Binding to one context does not disturb existing binds to other contexts.
			PipelineContext pipeline_context;
		};

		/**
		 * Record a bind of one of more @ref DescriptorSet
		 * See @ref CommandBufferRecording::bind_descriptor_sets for usage.
		 */
		struct BindDescriptorSets
		{
			/// PipelineLayout representing the list of layouts matching each provided set.
			const PipelineLayout* pipeline_layout;
			/// Pipeline bind point.
			PipelineContext context;
			/// List of DescriptorSet to bind.
			const tz::BasicList<const DescriptorSet*> descriptor_sets;
			/// The operation causes the sets [first_set_id, first_set_id + descriptor_sets.length() + 1] to refer to the DescriptorSets within `descriptor_sets`.
			std::uint32_t first_set_id;
		};

		/**
		 * Record a beginning of some @ref RenderPass.
		 * See @ref CommandBufferRecording::RenderPassRun for usage.
		 */
		struct BeginRenderPass
		{
			/// The @ref RenderPass which will begin.
			const RenderPass* pass;
		};
		
		/**
		 * Record the ending of some @ref RenderPass.
		 * See @ref CommandBufferRecording::RenderPassRun for usage.
		 */
		struct EndRenderPass
		{
			/// The @ref RenderPass which will end.
			const RenderPass* pass;
		};

		/**
		 * Record a copy from one @ref Buffer to another.
		 *
		 * The first N bytes will be copied from the source buffer to the destination buffer, where `N == min(src->size(), dst->size())`
		 */
		struct BufferCopyBuffer
		{
			/// Buffer to copy data from. Must not be null.
			const Buffer* src;
			/// Buffer to copy to. Must not be null.
			Buffer* dst;
		};

		/**
		 * Bind a vertex or index @ref Buffer.
		 */
		struct BindBuffer
		{
			/// Buffer to bind, usage must contain *one* of @ref BufferUsage::VertexBuffer or @ref BufferUsage::IndexBuffer.
			const Buffer* buffer;
		};

		/**
		 * Transition an image layout via recording a pipeline barrier into the @ref CommandBuffer.
		 *
		 * @post After submission, `image->get_layout() == .target_layout` is guaranteed to be true, however this may actually occur *before* some @ref hardware::Queue has actually fully invoked this command. For that reason, if `image->get_layout()` is accessed before the submission of this command has fully finished (see @ref hardware::Queue::SubmitInfo::execution_complete_fence), then the behaviour is undefined.
		 * @internal See @ref hardware::queue::execute_cpu_side_command_buffer for implementation details of the changing of the @ref ImageLayout CPU-side.
		 */
		struct TransitionImageLayout
		{
			/// Image to transition.
			Image* image;
			/// Desired ImageLayout of `image` after the command has finished execution.
			ImageLayout target_layout;

			/// All preceeding operations within the recorded @ref CommandBuffer that satisfy an access method specified here will not be re-ordered (guaranteed to be before every candidate after the barrier)
			AccessFlagField source_access;
			/// All succeding operations within the recorded @ref CommandBuffer that satisfy an access method specified here will not be re-ordered (guaranteed to be after every candidate before the barrier).
			AccessFlagField destination_access;
			/// Stage that the barrier blocks after.
			PipelineStage source_stage;
			/// Stage that the barrier blocks before.
			PipelineStage destination_stage;
			/// Aspect of `image` in this context.
			ImageAspectFlags image_aspects;

			/// List of affected mip levels. Must be in ascending-order and with no gaps. E.g {0, 1, 2} is fine, {0, 2, 3} is not and neither is {0, 2, 1}. Default is {0}.
			tz::BasicList<std::uint32_t> affected_mip_levels = {0u};
			/// List of affected array layers. Must be in ascending-order and with no gaps. E.g {0, 1, 2} is fine, {0, 2, 3} is not and neither is {0, 2, 1}. Default is {0}.
			tz::BasicList<std::uint32_t> affected_layers = {0u};
		};

		/// Variant type which has alternatives for every single possible recordable command type.
		using Variant = std::variant<Draw, BindPipeline, BindDescriptorSets, BeginRenderPass, EndRenderPass, BufferCopyBuffer, BindBuffer, TransitionImageLayout>;
	};
	/**
	 * @ingroup tz_gl_vk_commands
	 * Specifies creation flags for a @ref CommandPool.
	 */
	struct CommandPoolInfo
	{
		/// Specifies which hardware queue is expected to act as the executor for the CommandPool's buffers.
		const hardware::Queue* queue;
	};

	class CommandBuffer;

	/**
	 * @ingroup tz_gl_vk_commands
	 * Represents the full duration of the recording process of an existing @ref CommandBuffer.
	 */
	class CommandBufferRecording
	{
	public:
		/**
		 * Represents the full duration of an invocation of a @ref RenderPass during a @ref CommandBufferRecording.
		 * @note Any vulkan recording commands invoked during the lifetime of this object can apply to the corresponding @ref RenderPass.
		 */
		class RenderPassRun
		{
		public:
			/**
			 * Record the beginning of the @ref RenderPass sourcing the provided @ref Framebuffer.
			 * @param framebuffer Framebuffer whose @ref RenderPass should begin within the @ref CommandBuffer.
			 * @param recording Existing recording of a @ref CommandBuffer which shall record the beginning/ending of the render pass.
			 * @note The construction of this object begins the render pass, and the destruction ends the render pass. This means all commands recorded during the lifetime of this object will apply to the render pass.
			 */
			RenderPassRun(const Framebuffer& framebuffer, CommandBufferRecording& recording);
			RenderPassRun(const RenderPassRun& copy) = delete;
			RenderPassRun(RenderPassRun&& move) = delete;
			~RenderPassRun();

			RenderPassRun& operator=(const RenderPassRun& rhs) = delete;
			RenderPassRun& operator=(RenderPassRun&& rhs) = delete;
		private:
			const Framebuffer* framebuffer;
			CommandBufferRecording* recording;
		};

		CommandBufferRecording(CommandBuffer& command_buffer);
		CommandBufferRecording(const CommandBufferRecording& copy) = delete;
		CommandBufferRecording(CommandBufferRecording&& move);
		~CommandBufferRecording();

		CommandBufferRecording& operator=(const CommandBufferRecording& rhs) = delete;
		CommandBufferRecording& operator=(CommandBufferRecording&& rhs);

		/**
		 * Bind an existing @ref GraphicsPipeline to some context.
		 * See @ref VulkanCommand::BindPipeline for details.
		 */
		void bind_pipeline(VulkanCommand::BindPipeline command);
		/**
		 * Perform a non-instanced draw.
		 * See @ref VulkanCommand::Draw for details.
		 */
		void draw(VulkanCommand::Draw draw);
		/**
		 * Bind a list of @ref DescriptorSet.
		 * See @ref VulkanCommand::BindDescriptorSets for details.
		 */
		void bind_descriptor_sets(VulkanCommand::BindDescriptorSets command);
		/**
		 * Copy data from one @ref Buffer to another.
		 * See @ref VulkanCommand::BufferCopyBuffer for details.
		 */
		void buffer_copy_buffer(VulkanCommand::BufferCopyBuffer command);
		/**
		 * Bind a @ref Buffer.
		 * See @ref VulkanCommand::BindBuffer for details.
		 */
		void bind_buffer(VulkanCommand::BindBuffer command);

		void transition_image_layout(VulkanCommand::TransitionImageLayout command);

		/**
		 * Retrieve the @ref CommandBuffer that is currently being recorded.
		 * @return CommandBuffer that this recording corresponds to.
		 */
		const CommandBuffer& get_command_buffer() const;

		friend class RenderPassRun;
	private:
		CommandBuffer& get_command_buffer();
		void register_command(VulkanCommand::Variant command);

		CommandBuffer* command_buffer;
	};

	class CommandPool;

	/**
	 * @ingroup tz_gl_vk_commands
	 * Represents storage for vulkan commands, such as draw calls, binds, transfers etc...
	 */
	class CommandBuffer
	{
	public:
		friend class CommandPool;
		/**
		 * CommandBuffers are allocated from a @ref CommandPool. Each pool was spawned from a @ref LogicalDevice. Retrieve a reference to the LogicalDevice which spawned the pool which owns this buffer.
		 */
		const LogicalDevice& get_device() const;
		/**
		 * Begin recording the CommandBuffer.
		 * @pre The CommandBuffer is not currently being recorded. See @ref CommandBuffer::is_recording
		 * @return An object which can be used to invoke vulkan commands. The duration of the recording matches that of the returned object.
		 */
		CommandBufferRecording record();
		/**
		 * Query as to whether this CommandBuffer is currently recording.
		 */
		bool is_recording() const;
		/**
		 * Retrieve the number of commands recorded into the buffer.
		 * @return Number of commands within the buffer.
		 */
		std::size_t command_count() const;
		std::span<const VulkanCommand::Variant> get_recorded_commands() const;

		using NativeType = VkCommandBuffer;
		NativeType native() const;

		friend class CommandBufferRecording;
	private:
		void set_recording(bool recording);
		void add_command(VulkanCommand::Variant command);

		CommandBuffer(const CommandPool& owner_pool, CommandBuffer::NativeType native);

		VkCommandBuffer command_buffer;
		const CommandPool* owner_pool;
		bool recording;
		std::vector<VulkanCommand::Variant> recorded_commands;
	};

	/**
	 * @ingroup tz_gl_vk_commands
	 * Represents storage for CommandBuffers.
	 */
	class CommandPool
	{
	public:
		/**
		 * Specifies information about an allocation of a @ref CommandBuffer or many.
		 */
		struct Allocation
		{
			/// Number of CommandBuffers to create.
			std::uint32_t buffer_count;
		};

		/**
		 * Contains information about the result of a pool allocation. See @ref CommandPool::allocate_buffers.
		 */
		struct AllocationResult
		{
			/// Query as to whether the allocation was successful or not.
			bool success() const;

			/// Describes how well an allocation went.
			enum class AllocationResultType
			{
				Success,
				FatalError
			};

			/// List of newly-allocated buffers.
			tz::BasicList<CommandBuffer> buffers;
			/// Result description for this specific allocation.
			AllocationResultType type;
		};

		CommandPool(CommandPoolInfo info);
		CommandPool(const CommandPool& copy) = delete;
		CommandPool(CommandPool&& move);
		~CommandPool();

		CommandPool& operator=(const CommandPool& rhs) = delete;
		CommandPool& operator=(CommandPool&& rhs);

		const LogicalDevice& get_device() const;

		/**
		 * Allocate some CommandBuffers. See @ref Allocation for more info.
		 * @return Structure with the resultant information, including newly-allocated CommandBuffers.
		 */
		AllocationResult allocate_buffers(const Allocation& alloc);

		using NativeType = VkCommandPool;
		NativeType native() const;
	private:
		VkCommandPool pool;
		CommandPoolInfo info;
	};
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_IMPL_BACKEND_VK2_COMMAND_HPP
