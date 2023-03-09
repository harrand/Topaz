#ifndef TOPAZ_GL_IMPL_BACKEND_VK2_COMMAND_HPP
#define TOPAZ_GL_IMPL_BACKEND_VK2_COMMAND_HPP
#if TZ_VULKAN
#include "tz/gl/impl/vulkan/detail/render_pass.hpp"
#include "tz/gl/impl/vulkan/detail/hardware/queue.hpp"
#include "tz/gl/impl/vulkan/detail/logical_device.hpp"
#include "tz/gl/impl/vulkan/detail/framebuffer.hpp"
#include "tz/gl/impl/vulkan/detail/graphics_pipeline.hpp"
#include "tz/gl/impl/vulkan/detail/buffer.hpp"

namespace tz::gl::vk2
{
	/**
	 * @ingroup tz_gl_vk_commands
	 * Contains all the possible commands which can be recorded within a @ref CommandBuffer.
	 */
	struct VulkanCommand
	{
		struct Dispatch
		{
			tz::vec3ui groups;
		};
		/**
		 * Record a non-indexed draw.
		 * See @ref CommandBufferRecording::draw for usage.
		 */
		struct Draw
		{
			/// number of vertices to draw.
			std::uint32_t vertex_count;
			/// number of instances to draw (Default 1).
			std::uint32_t instance_count = 1;
			/// Index of the first vertex to draw (Default 0).
			std::uint32_t first_vertex = 0;
			/// Instance ID of the first instance to draw (Default 0).
			std::uint32_t first_instance = 0;
		};

		/**
		 * Record an indexed draw.
		 * See @ref CommandBufferRecording::draw_indexed for usage.
		 */
		struct DrawIndexed
		{
			std::uint32_t index_count;
			std::uint32_t instance_count = 1;
			std::uint32_t first_index = 0;
			std::int32_t vertex_offset = 0;
			std::uint32_t first_instance = 0;
		};

		/**
		 * Record an unindexed indirect draw.
		 * See @ref COmmandBufferRecording::draw_indirect for usage.
		 */
		struct DrawIndirect
		{
			const Buffer* draw_indirect_buffer;
			std::uint32_t draw_count;
			std::uint32_t stride;
			VkDeviceSize offset = 0;
		};

		struct DrawIndirectCount
		{
			const Buffer* draw_indirect_buffer;
			std::uint32_t max_draw_count;
			std::uint32_t stride;
			VkDeviceSize offset = sizeof(std::uint32_t);
		};

		/**
		 * Record an indexed indirect draw.
		 * See @ref COmmandBufferRecording::draw_indexed_indirect for usage.
		 */
		struct DrawIndexedIndirect
		{
			const Buffer* draw_indirect_buffer;
			std::uint32_t draw_count;
			std::uint32_t stride;
			VkDeviceSize offset = 0;
		};

		struct DrawIndexedIndirectCount
		{
			const Buffer* draw_indirect_buffer;
			std::uint32_t max_draw_count;
			std::uint32_t stride;
			VkDeviceSize offset = sizeof(std::uint32_t);
		};

		/**
		 * Bind an index buffer.
		 * See @ref CommandBufferRecording::bind_index_buffer for usage.
		 */
		struct BindIndexBuffer
		{
			const vk2::Buffer* index_buffer;
			std::uint64_t buffer_offset = 0;
		};

		/**
		 * Record a bind of a @ref GraphicsPipeline.
		 * See @ref CommandBufferRecording::bind_pipeline for usage.
		 */
		struct BindPipeline
		{
			/// Pipeline to be bound. Must not be null.
			const Pipeline* pipeline;
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
			const tz::basic_list<const DescriptorSet*> descriptor_sets;
			/// The operation causes the sets [first_set_id, first_set_id + descriptor_sets.length() + 1] to refer to the DescriptorSets within `descriptor_sets`.
			std::uint32_t first_set_id;
		};

		/**
		 * Record a beginning of some @ref RenderPass.
		 * See @ref CommandBufferRecording::RenderPassRun for usage.
		 */
		struct BeginRenderPass
		{
			/// Framebuffer containing the @ref RenderPass.
			Framebuffer* framebuffer;
		};

		struct BeginDynamicRendering{};

		struct EndDynamicRendering{};
		
		/**
		 * Record the ending of some @ref RenderPass.
		 * See @ref CommandBufferRecording::RenderPassRun for usage.
		 */
		struct EndRenderPass
		{
			/// Framebuffer containing the @ref RenderPass.
			Framebuffer* framebuffer;
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
			/// Offset, in bytes, from the beginning of the source buffer to copy from.
			std::size_t src_offset = 0;
			/// Offset, in bytes, from the beginning of the destination buffer to copy to.
			std::size_t dst_offset = 0;
		};

		/**
		 * Record a copy from one @ref Buffer to an @ref Image.
		 *
		 * The first N bytes will be copied from the source buffer to the destination image, there N == min(src->size(), dst->size()).
		 */
		struct BufferCopyImage
		{
			/// Buffer to copy data from. Must not be null.
			const Buffer* src;
			/// Buffer to copy to. Must not be null.
			Image* dst;
			/// Aspect of `image` in this context.
			ImageAspectFlags image_aspects;
		};

		/**
		 * Copy data from one image to another.
		 *
		 * Note: If image sizes do not match, the minimums of their dimensions are represented as a subregion to copy over, starting at [0, 0] for each image.
		 */
		struct ImageCopyImage
		{
			/// Image to copy from
			const Image* src;
			/// Image to copy to
			Image* dst;
			/// Aspect of both images.
			ImageAspectFlags image_aspects;
		};

		/**
		 * Bind a vertex or index @ref Buffer.
		 */
		struct BindBuffer
		{
			/// Buffer to bind, usage must contain *one* of @ref BufferUsage::VertexBuffer or @ref BufferUsage::index_buffer.
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
			tz::basic_list<std::uint32_t> affected_mip_levels = {0u};
			/// List of affected array layers. Must be in ascending-order and with no gaps. E.g {0, 1, 2} is fine, {0, 2, 3} is not and neither is {0, 2, 1}. Default is {0}.
			tz::basic_list<std::uint32_t> affected_layers = {0u};
		};

		struct SetScissorDynamic
		{
			tz::vec2ui offset;
			tz::vec2ui extent;
		};

		struct DebugBeginLabel
		{
			std::string name;
			tz::vec4 colour{0.0f, 0.0f, 0.0f, 0.0f};
		};

		struct DebugEndLabel{};

		/// variant type which has alternatives for every single possible recordable command type.
		using variant = std::variant<Dispatch, Draw, DrawIndexed, DrawIndirect, DrawIndirectCount, DrawIndexedIndirect, DrawIndexedIndirectCount, BindIndexBuffer, BindPipeline, BindDescriptorSets, BeginRenderPass, EndRenderPass, BeginDynamicRendering, EndDynamicRendering, BufferCopyBuffer, BufferCopyImage, ImageCopyImage, BindBuffer, TransitionImageLayout, SetScissorDynamic, DebugBeginLabel, DebugEndLabel>;
	};

	enum class CommandPoolFlag
	{
		Reusable = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
	};
	using CommandPoolFlags = tz::enum_field<CommandPoolFlag>;
	/**
	 * @ingroup tz_gl_vk_commands
	 * Specifies creation flags for a @ref CommandPool.
	 */
	struct CommandPoolInfo
	{
		/// Specifies which hardware queue is expected to act as the executor for the CommandPool's buffers.
		const hardware::Queue* queue;
		/// Specifies extra flags which affect the created command pool.
		CommandPoolFlags flags = {};
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
			RenderPassRun(Framebuffer& framebuffer, CommandBufferRecording& recording, tz::vec4 clear_colour = {0.0f, 0.0f, 0.0f, 1.0f});
			RenderPassRun(const RenderPassRun& copy) = delete;
			RenderPassRun(RenderPassRun&& move) = delete;
			~RenderPassRun();

			RenderPassRun& operator=(const RenderPassRun& rhs) = delete;
			RenderPassRun& operator=(RenderPassRun&& rhs) = delete;
		private:
			Framebuffer* framebuffer;
			CommandBufferRecording* recording;
		};

		class DynamicRenderingRun
		{
		public:
			DynamicRenderingRun(CommandBufferRecording& record, std::span<const vk2::ImageView> colour_attachments, const vk2::ImageView* depth_attachment);
			~DynamicRenderingRun();

			DynamicRenderingRun& operator=(const DynamicRenderingRun& rhs) = delete;
			DynamicRenderingRun& operator=(DynamicRenderingRun&& rhs) = delete;
		private:
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
		 * Dispatch some compute work.
		 * See @ref VulkanCommand::Dispatch for details.
		 */
		void dispatch(VulkanCommand::Dispatch dispatch);
		/**
		 * Perform a non-instanced draw.
		 * See @ref VulkanCommand::Draw for details.
		 */
		void draw(VulkanCommand::Draw draw);
		/**
		 * Perform an indexed draw.
		 * See @ref VulkanCommand::Draw for details.
		 */
		void draw_indexed(VulkanCommand::DrawIndexed draw);
		/**
		 * Perform some indirect, non-indexed draws.
		 * See @ref VulkanCommand::DrawIndirect for details.
		 */
		void draw_indirect(VulkanCommand::DrawIndirect draw);
		void draw_indirect_count(VulkanCommand::DrawIndirectCount draw);
		/**
		 * Perform some indirect, indexed draws.
		 * See @ref VulkanCommand::DrawIndexedIndirect for details.
		 */
		void draw_indexed_indirect(VulkanCommand::DrawIndexedIndirect draw);
		void draw_indexed_indirect_count(VulkanCommand::DrawIndexedIndirectCount draw);
		void bind_index_buffer(VulkanCommand::BindIndexBuffer bind);
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
		 * Copy data from one @ref Buffer to an @ref Image.
		 * See @ref VulkanCommand::BufferCopyImage for details.
		 */
		void buffer_copy_image(VulkanCommand::BufferCopyImage command);
		/**
		 * Copy data from one @ref Image to another.
		 * See @ref VulkanCommand::ImageCopyImage for details.
		 */
		void image_copy_image(VulkanCommand::ImageCopyImage command);
		/**
		 * Bind a @ref Buffer.
		 * See @ref VulkanCommand::BindBuffer for details.
		 */
		void bind_buffer(VulkanCommand::BindBuffer command);

		void transition_image_layout(VulkanCommand::TransitionImageLayout command);

		void set_scissor_dynamic(VulkanCommand::SetScissorDynamic command);

		void debug_begin_label(VulkanCommand::DebugBeginLabel command);
		void debug_end_label(VulkanCommand::DebugEndLabel command);

		/**
		 * Retrieve the @ref CommandBuffer that is currently being recorded.
		 * @return CommandBuffer that this recording corresponds to.
		 */
		const CommandBuffer& get_command_buffer() const;

		friend class RenderPassRun;
	private:
		CommandBuffer& get_command_buffer();
		void register_command(VulkanCommand::variant command);
		ImageLayout get_layout_so_far(const Image& image) const;

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
		 * @return number of commands within the buffer.
		 */
		std::size_t command_count() const;
		std::span<const VulkanCommand::variant> get_recorded_commands() const;
		void set_owner(const CommandPool& owner);

		using NativeType = VkCommandBuffer;
		NativeType native() const;

		friend class CommandBufferRecording;
	private:
		void set_recording(bool recording);
		void add_command(VulkanCommand::variant command);

		CommandBuffer(const CommandPool& owner_pool, CommandBuffer::NativeType native);

		VkCommandBuffer command_buffer;
		const CommandPool* owner_pool;
		bool recording;
		std::vector<VulkanCommand::variant> recorded_commands;
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
			/// number of CommandBuffers to create.
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
				AllocationSuccess,
				FatalError
			};

			/// List of newly-allocated buffers.
			tz::basic_list<CommandBuffer> buffers;
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
		void free_buffers(const AllocationResult& alloc_result);

		using NativeType = VkCommandPool;
		NativeType native() const;

		static CommandPool null();
		bool is_null() const;
	private:
		CommandPool();

		VkCommandPool pool;
		CommandPoolInfo info;
	};

	struct CommandBufferData
	{
		CommandBufferData() = default;
		CommandBufferData(CommandBufferData&& move)
		{
			*this = std::move(move);
		}
		CommandBufferData& operator=(CommandBufferData&& rhs)
		{
			std::swap(this->pool, rhs.pool);
			std::swap(this->data, rhs.data);
			for(auto& buf : this->data.buffers)
			{
				buf.set_owner(this->pool);
			}
			for(auto& buf : rhs.data.buffers)
			{
				buf.set_owner(rhs.pool);
			}
			return *this;
		}
		vk2::CommandPool pool = vk2::CommandPool::null();
		vk2::CommandPool::AllocationResult data = {};
	};
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_IMPL_BACKEND_VK2_COMMAND_HPP
