#ifndef TOPAZ_GL_IMPL_BACKEND_VK2_COMMAND_HPP
#define TOPAZ_GL_IMPL_BACKEND_VK2_COMMAND_HPP
#if TZ_VULKAN
#include "gl/impl/backend/vk2/hardware/queue.hpp"
#include "gl/impl/backend/vk2/logical_device.hpp"

namespace tz::gl::vk2
{
	/**
	 * @ingroup tz_gl_vk_commands
	 * Specifies creation flags for a @ref CommandPool.
	 */
	struct CommandPoolInfo
	{
		/// Specifies which hardware queue is expected to act as the executor for the CommandPool's buffers.
		const hardware::Queue* queue;
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

		using NativeType = VkCommandBuffer;
		NativeType native() const;
	private:
		CommandBuffer(const CommandPool& owner_pool, CommandBuffer::NativeType native);

		VkCommandBuffer command_buffer;
		const CommandPool* owner_pool;
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
