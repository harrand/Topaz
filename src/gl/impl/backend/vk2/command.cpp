#if TZ_VULKAN
#include "gl/impl/backend/vk2/command.hpp"

namespace tz::gl::vk2
{
	CommandBufferRecording::RenderPassRun::RenderPassRun(const Framebuffer& framebuffer, const CommandBufferRecording& recording):
	framebuffer(&framebuffer),
	recording(&recording)
	{
		std::vector<VkClearValue> clear_values;

		VkRenderPassBeginInfo begin
		{
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
			.pNext = nullptr,
			.renderPass = this->framebuffer->get_pass().native(),
			.framebuffer = this->framebuffer->native(),
			.renderArea =
			{
				.offset = {0, 0},
				.extent =
				{
					.width = this->framebuffer->get_dimensions()[0],
					.height = this->framebuffer->get_dimensions()[1]
				}
			},
			.clearValueCount = static_cast<std::uint32_t>(clear_values.size()),
			.pClearValues = clear_values.data()
		};

		vkCmdBeginRenderPass(this->recording->get_command_buffer().native(), &begin, VK_SUBPASS_CONTENTS_INLINE);
	}

	CommandBufferRecording::RenderPassRun::~RenderPassRun()
	{
		vkCmdEndRenderPass(this->recording->get_command_buffer().native());
	}

	CommandBufferRecording::CommandBufferRecording(CommandBuffer& command_buffer):
	command_buffer(&command_buffer)
	{
		VkCommandBufferBeginInfo begin
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = nullptr,
			.flags = 0,
			.pInheritanceInfo = nullptr
		};
		VkResult res = vkBeginCommandBuffer(this->command_buffer->native(), &begin);
		switch(res)
		{
			case VK_SUCCESS:
				this->command_buffer->set_recording(true);
			break;
			case VK_ERROR_OUT_OF_HOST_MEMORY:
				tz_error("Failed to create CommandBufferRecording because we ran out of host memory (RAM). Please ensure that your system meets the minimum requirements.");
			break;
			case VK_ERROR_OUT_OF_DEVICE_MEMORY:
				tz_error("Failed to create CommandBufferRecording because we ran out of device memory (VRAM). Please ensure that your system meets the minimum requirements.");
			break;
			default:
				tz_error("Failed to create CommandBufferRecording but cannot determine why. Please submit a bug report.");
			break;
		}
	}

	CommandBufferRecording::CommandBufferRecording(CommandBufferRecording&& move):
	command_buffer(nullptr)
	{
		*this = std::move(move);
	}

	CommandBufferRecording::~CommandBufferRecording()
	{
		if(this->command_buffer != nullptr)
		{
			vkEndCommandBuffer(this->command_buffer->native());
			this->command_buffer->set_recording(false);
		}
	}

	CommandBufferRecording& CommandBufferRecording::operator=(CommandBufferRecording&& rhs)
	{
		std::swap(this->command_buffer, rhs.command_buffer);
		return *this;
	}

	const CommandBuffer& CommandBufferRecording::get_command_buffer() const
	{
		tz_assert(this->command_buffer != nullptr, "CommandBufferRecording had nullptr CommandBuffer. Please submit a bug report");
		return *this->command_buffer;
	}

	const LogicalDevice& CommandBuffer::get_device() const
	{
		return this->owner_pool->get_device();
	}

	CommandBufferRecording CommandBuffer::record()
	{
		tz_assert(!this->recording, "CommandBuffer already being recorded");
		return {*this};
	}

	CommandBuffer::NativeType CommandBuffer::native() const
	{
		return this->command_buffer;
	}

	void CommandBuffer::set_recording(bool recording)
	{
		this->recording = recording;
	}
	
	CommandBuffer::CommandBuffer(const CommandPool& owner_pool, CommandBuffer::NativeType native):
	command_buffer(native),
	owner_pool(&owner_pool),
	recording(false)
	{

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
