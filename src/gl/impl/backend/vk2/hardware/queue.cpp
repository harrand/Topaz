#include "gl/impl/backend/vk2/command.hpp"
#if TZ_VULKAN
#include "gl/impl/backend/vk2/hardware/queue.hpp"
#include "gl/impl/backend/vk2/semaphore.hpp"

namespace tz::gl::vk2::hardware
{
	Queue::Queue(QueueInfo info):
	queue(VK_NULL_HANDLE),
	info(info)
	{
		tz_assert(info.dev != nullptr, "QueueInfo's LogicalDevice was nullptr");
		vkGetDeviceQueue(info.dev->native(), this->info.queue_family_idx, this->info.queue_idx, &this->queue);
	}

	const QueueInfo& Queue::get_info() const
	{
		return this->info;
	}

	const LogicalDevice& Queue::get_device() const
	{
		tz_assert(this->info.dev != nullptr, "QueueInfo contained nullptr LogicalDevice");
		return *this->info.dev;
	}

	void Queue::submit(SubmitInfo submit_info)
	{
		std::vector<BinarySemaphore::NativeType> wait_sem_natives(submit_info.waits.length());
		std::vector<VkPipelineStageFlags> wait_stage_natives(submit_info.waits.length());
		for(std::size_t i = 0; i < submit_info.waits.length(); i++)
		{
			wait_sem_natives[i] = submit_info.waits[i].wait_semaphore->native();
			wait_stage_natives[i] = static_cast<VkPipelineStageFlags>(submit_info.waits[i].wait_stage);
		}

		std::vector<CommandBuffer::NativeType> command_buffer_natives(submit_info.command_buffers.length());
		std::transform(submit_info.command_buffers.begin(), submit_info.command_buffers.end(), command_buffer_natives.begin(), [](const CommandBuffer* buf){return buf->native();});

		std::vector<BinarySemaphore::NativeType> signal_sem_natives(submit_info.signal_semaphores.length());
		std::transform(submit_info.signal_semaphores.begin(), submit_info.signal_semaphores.end(), signal_sem_natives.begin(), [](const BinarySemaphore* signal_sem){return signal_sem->native();});

		VkSubmitInfo sub
		{
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.pNext = nullptr,
			.waitSemaphoreCount = static_cast<std::uint32_t>(submit_info.waits.length()),
			.pWaitSemaphores = wait_sem_natives.data(),
			.pWaitDstStageMask = wait_stage_natives.data(),
			.commandBufferCount = static_cast<std::uint32_t>(submit_info.command_buffers.length()),
			.pCommandBuffers = command_buffer_natives.data(),
			.signalSemaphoreCount = static_cast<std::uint32_t>(submit_info.signal_semaphores.length()),
			.pSignalSemaphores = signal_sem_natives.data()
		};

		VkResult res = vkQueueSubmit(this->queue, 1, &sub, VK_NULL_HANDLE);
		switch(res)
		{
			case VK_SUCCESS:
				// do nothing
			break;
			case VK_ERROR_OUT_OF_HOST_MEMORY:
				tz_error("Failed to submit Queue because we ran out of host memory (RAM). Please ensure that your system meets the minimum requirements.");
			break;
			case VK_ERROR_OUT_OF_DEVICE_MEMORY:
				tz_error("Failed to submit Queue because we ran out of device memory (VRAM). Please ensure that your system meets the minimum requirements.");
			break;
			case VK_ERROR_DEVICE_LOST:
				tz_error("Failed to submit Queue because device was lost. This is a fatal error.");
			break;
			default:
				tz_error("Failed to submit Queue but cannot determine why. Please submit a bug report.");
			break;
		}
	}

	Queue::NativeType Queue::native() const
	{
		return this->queue;
	}
}

#endif // TZ_VULKAN
