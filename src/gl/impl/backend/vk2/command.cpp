#if TZ_VULKAN
#include "gl/impl/backend/vk2/command.hpp"

namespace tz::gl::vk2
{
	CommandBuffer::CommandBuffer(const CommandPool& owner_pool, CommandBuffer::NativeType native):
	command_buffer(native),
	owner_pool(&owner_pool)
	{

	}

	const LogicalDevice& CommandBuffer::get_device() const
	{
		return this->owner_pool->get_device();
	}

	bool CommandPool::AllocationResult::success() const
	{
		return this->type == CommandPool::AllocationResult::AllocationResultType::Success;
	}

	CommandPool::CommandPool(CommandPoolInfo info):
	pool(VK_NULL_HANDLE),
	info(info)
	{
		VkCommandPoolCreateInfo create
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.queueFamilyIndex = this->info.queue->get_info().queue_family_idx
		};

		VkResult res = vkCreateCommandPool(this->get_device().native(), &create, nullptr, &this->pool);
		switch(res)
		{
			case VK_SUCCESS:
				// do nothing
			break;
			case VK_ERROR_OUT_OF_HOST_MEMORY:
				tz_error("Failed to create CommandPool because we ran out of host memory (RAM). Please ensure that your system meets the minimum requirements.");
			break;
			case VK_ERROR_OUT_OF_DEVICE_MEMORY:
				tz_error("Failed to create CommandPool because we ran out of device memory (VRAM). Please ensure that your system meets the minimum requirements.");
			break;
			default:
				tz_error("Failed to create CommandPool but cannot determine why. Please submit a bug report.");
			break;
		}
	}

	CommandPool::CommandPool(CommandPool&& move):
	pool(VK_NULL_HANDLE),
	info()
	{
		*this = std::move(move);
	}

	CommandPool::~CommandPool()
	{
		if(this->pool != VK_NULL_HANDLE)
		{
			vkDestroyCommandPool(this->get_device().native(), this->pool, nullptr);
			this->pool = VK_NULL_HANDLE;
		}
	}

	CommandPool& CommandPool::operator=(CommandPool&& rhs)
	{
		std::swap(this->pool, rhs.pool);
		std::swap(this->info, rhs.info);
		return *this;
	}

	const LogicalDevice& CommandPool::get_device() const
	{
		return this->info.queue->get_device();
	}

	CommandPool::AllocationResult CommandPool::allocate_buffers(const Allocation& alloc)
	{
		VkCommandBufferAllocateInfo create
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.pNext = nullptr,
			.commandPool = this->pool,
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = alloc.buffer_count
		};

		std::vector<CommandBuffer::NativeType> cmd_buf_natives(alloc.buffer_count);

		VkResult res = vkAllocateCommandBuffers(this->get_device().native(), &create, cmd_buf_natives.data());
		CommandPool::AllocationResult alloc_res;
		switch(res)
		{
			case VK_SUCCESS:
				alloc_res.type = CommandPool::AllocationResult::AllocationResultType::Success;
				for(CommandBuffer::NativeType cmd_buf_native : cmd_buf_natives)
				{
					alloc_res.buffers.add({*this, cmd_buf_native});
				}
			break;
			default:
				alloc_res.type = CommandPool::AllocationResult::AllocationResultType::FatalError;
			break;
		}

		return alloc_res;
	}

	CommandPool::NativeType CommandPool::native() const
	{
		return this->pool;
	}
}

#endif // TZ_VULKAN
