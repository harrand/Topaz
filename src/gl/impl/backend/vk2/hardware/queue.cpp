#if TZ_VULKAN
#include "gl/impl/backend/vk2/command.hpp"
#include "gl/impl/backend/vk2/hardware/queue.hpp"
#include "gl/impl/backend/vk2/semaphore.hpp"
#include "gl/impl/backend/vk2/fence.hpp"
#include "gl/impl/backend/vk2/swapchain.hpp"

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

	void Queue::set_logical_device(const LogicalDevice& device)
	{
		this->info.dev = &device;
	}

	void Queue::submit(SubmitInfo submit_info)
	{
		if(submit_info.execution_complete_fence != nullptr)
		{
			tz_assert(!submit_info.execution_complete_fence->is_signalled(), "SubmitInfo contained Fence to signal on execution complete, but it was already signalled.");
		}

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

		VkFence fence_native = VK_NULL_HANDLE;
		if(submit_info.execution_complete_fence != nullptr)
		{
			fence_native = submit_info.execution_complete_fence->native();
		}
		VkResult res = vkQueueSubmit(this->queue, 1, &sub, fence_native);
		switch(res)
		{
			case VK_SUCCESS:
				// Some commands require cpu-side operations too.
				for(const CommandBuffer* buf : submit_info.command_buffers)
				{
					this->execute_cpu_side_command_buffer(*buf);
				}
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

	Queue::PresentResult Queue::present(Queue::PresentInfo present_info)
	{
		std::vector<BinarySemaphore::NativeType> wait_sem_natives(present_info.wait_semaphores.length());
		
		std::transform(present_info.wait_semaphores.begin(), present_info.wait_semaphores.end(), wait_sem_natives.begin(), [](const BinarySemaphore* sem){return sem->native();});

		Swapchain::NativeType swapchain_native = present_info.swapchain->native();

		VkPresentInfoKHR present
		{
			.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			.pNext = nullptr,
			.waitSemaphoreCount = static_cast<std::uint32_t>(present_info.wait_semaphores.length()),
			.pWaitSemaphores = wait_sem_natives.data(),
			.swapchainCount = 1,
			.pSwapchains = &swapchain_native,
			.pImageIndices = &present_info.swapchain_image_index,
			.pResults = nullptr
		};

		VkResult res = vkQueuePresentKHR(this->queue, &present);
		PresentResult pres_result;
		switch(res)
		{
			case VK_SUCCESS:
				pres_result = PresentResult::Success;
			break;
			case VK_SUBOPTIMAL_KHR:
				pres_result = PresentResult::Success_Suboptimal;
			break;
			case VK_ERROR_OUT_OF_HOST_MEMORY:
				pres_result = PresentResult::Fail_FatalError;
				tz_error("Failed to present Queue because we ran out of host memory (RAM). Please ensure that your system meets the minimum requirements.");
			break;
			case VK_ERROR_OUT_OF_DEVICE_MEMORY:
				pres_result = PresentResult::Fail_FatalError;
				tz_error("Failed to present Queue because we ran out of device memory (VRAM). Please ensure that your system meets the minimum requirements.");
			break;
			case VK_ERROR_DEVICE_LOST:
				pres_result = PresentResult::Fail_FatalError;
				tz_error("Failed to present Queue because device was lost. This is a fatal error.");
			break;
			case VK_ERROR_OUT_OF_DATE_KHR:
				pres_result = PresentResult::Fail_OutOfDate;
			break;
			case VK_ERROR_SURFACE_LOST_KHR:
				pres_result = PresentResult::Fail_SurfaceLost;
			break;
			case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
				pres_result = PresentResult::Fail_AccessDenied;
			break;
			default:
				pres_result = PresentResult::Fail_FatalError;
				tz_error("Failed to present Queue but cannot determine why. Please submit a bug report.");
			break;
		}
		return pres_result;
	}

	Queue::NativeType Queue::native() const
	{
		return this->queue;
	}

	void Queue::execute_cpu_side_command_buffer(const CommandBuffer& command_buffer) const
	{
		for(const VulkanCommand::Variant& cmd : command_buffer.get_recorded_commands())
		{
			std::visit([](auto&& val)
			{
				using T = std::decay_t<decltype(val)>;
				if constexpr(std::is_same_v<T, VulkanCommand::BeginRenderPass>)
				{
					// Change image's layout CPU-side.
					Framebuffer& framebuffer = *val.framebuffer;
					const RenderPass& pass = framebuffer.get_pass();
					for(std::size_t i = 0; i < framebuffer.get_attachment_views().length(); i++)
					{
						Image& img = framebuffer.get_attachment_views()[i]->get_image();
						ImageLayout output_layout = pass.get_info().attachments[i].initial_layout;
						img.set_layout(output_layout);
					}
				}
				if constexpr(std::is_same_v<T, VulkanCommand::EndRenderPass>)
				{
					// Change image's layout CPU-side.
					Framebuffer& framebuffer = *val.framebuffer;
					const RenderPass& pass = framebuffer.get_pass();
					for(std::size_t i = 0; i < framebuffer.get_attachment_views().length(); i++)
					{
						Image& img = framebuffer.get_attachment_views()[i]->get_image();
						ImageLayout output_layout = pass.get_info().attachments[i].final_layout;
						img.set_layout(output_layout);
					}
				}
				else if constexpr(std::is_same_v<T, VulkanCommand::TransitionImageLayout>)
				{
					// We probably need to change the image's layout CPU-side.
					Image& img = *val.image;
					img.set_layout(val.target_layout);
				}
			}, cmd);
		}
	}
}

#endif // TZ_VULKAN
