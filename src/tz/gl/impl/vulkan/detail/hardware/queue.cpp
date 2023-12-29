#if TZ_VULKAN
#include "tz/core/profile.hpp"
#include "tz/gl/impl/vulkan/detail/command.hpp"
#include "tz/gl/impl/vulkan/detail/hardware/queue.hpp"
#include "tz/gl/impl/vulkan/detail/semaphore.hpp"
#include "tz/gl/impl/vulkan/detail/fence.hpp"
#include "tz/gl/impl/vulkan/detail/swapchain.hpp"

namespace tz::gl::vk2::hardware
{
	Queue::Queue(QueueInfo info):
	queue(VK_NULL_HANDLE),
	info(info)
	{
		tz::assert(info.dev != nullptr, "QueueInfo's LogicalDevice was nullptr");
		vkGetDeviceQueue(info.dev->native(), this->info.queue_family_idx, this->info.queue_idx, &this->queue);
	}

	const QueueInfo& Queue::get_info() const
	{
		return this->info;
	}

	const LogicalDevice& Queue::get_device() const
	{
		tz::assert(this->info.dev != nullptr, "QueueInfo contained nullptr LogicalDevice");
		return *this->info.dev;
	}

	void Queue::set_logical_device(const LogicalDevice& device)
	{
		this->info.dev = &device;
	}

	void Queue::submit(SubmitInfo submit_info)
	{
		TZ_PROFZONE("Vulkan Backend - Queue Submit", 0xFFAA0000);
		if(submit_info.execution_complete_fence != nullptr)
		{
			tz::assert(!submit_info.execution_complete_fence->is_signalled(), "SubmitInfo contained Fence to signal on execution complete, but it was already signalled.");
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

		std::vector<BinarySemaphore::NativeType> signal_sem_natives(submit_info.signals.length());
		std::transform(submit_info.signals.begin(), submit_info.signals.end(), signal_sem_natives.begin(), [](const SubmitInfo::SignalInfo& signal){return signal.signal_semaphore->native();});

		std::vector<std::uint64_t> wait_timeline_values;
		std::vector<std::uint64_t> signal_timeline_values;
		bool using_timelines = std::any_of(submit_info.signals.begin(), submit_info.signals.end(), [](const auto& signal){return signal.signal_semaphore->get_type() == semaphore_type::timeline;})
							|| std::any_of(submit_info.waits.begin(), submit_info.waits.end(), [](const auto& wait){return wait.wait_semaphore->get_type() == semaphore_type::timeline;});
		if(using_timelines)
		{
			//tz::assert(std::all_of(submit_info.signals.begin(), submit_info.signals.end(), [](const auto& signal){return signal.signal_semaphore->get_type() == semaphore_type::timeline;}), "Found mix of binary and timeline semaphores in wait submit. Unfortunately, you can only use one or the other (feel free to submit a feature request)");
			wait_timeline_values.resize(submit_info.waits.length());
			std::transform(submit_info.waits.begin(), submit_info.waits.end(), wait_timeline_values.begin(), [](const auto& wait){return wait.timeline;});

			signal_timeline_values.resize(submit_info.signals.length());
			std::transform(submit_info.signals.begin(), submit_info.signals.end(), signal_timeline_values.begin(), [](const auto& signal){return signal.timeline;});
		}

		VkTimelineSemaphoreSubmitInfo timeline_sinfo
		{
			.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO,
			.pNext = nullptr,
			.waitSemaphoreValueCount = static_cast<std::uint32_t>(wait_timeline_values.size()),
			.pWaitSemaphoreValues = wait_timeline_values.data(),
			.signalSemaphoreValueCount = static_cast<std::uint32_t>(signal_timeline_values.size()),
			.pSignalSemaphoreValues = signal_timeline_values.data()
		};

		void* next = nullptr;
		if(using_timelines)
		{
			next = &timeline_sinfo;
		}

		VkSubmitInfo sub
		{
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.pNext = next,
			.waitSemaphoreCount = static_cast<std::uint32_t>(submit_info.waits.length()),
			.pWaitSemaphores = wait_sem_natives.data(),
			.pWaitDstStageMask = wait_stage_natives.data(),
			.commandBufferCount = static_cast<std::uint32_t>(submit_info.command_buffers.length()),
			.pCommandBuffers = command_buffer_natives.data(),
			.signalSemaphoreCount = static_cast<std::uint32_t>(submit_info.signals.length()),
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
				tz::error("Failed to submit Queue because we ran out of host memory (RAM). Please ensure that your system meets the minimum requirements.");
			break;
			case VK_ERROR_OUT_OF_DEVICE_MEMORY:
				tz::error("Failed to submit Queue because we ran out of device memory (VRAM). Please ensure that your system meets the minimum requirements.");
			break;
			case VK_ERROR_DEVICE_LOST:
				tz::error("Failed to submit Queue because device was lost. This is a fatal error.");
			break;
			default:
				tz::error("Failed to submit Queue but cannot determine why. Please submit a bug report.");
			break;
		}
	}

	Queue::PresentResult Queue::present(Queue::PresentInfo present_info)
	{
		TZ_PROFZONE("Vulkan Backend - Queue Present", 0xFFAA0000);
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
				pres_result = PresentResult::Success_NoIssue;
			break;
			case VK_SUBOPTIMAL_KHR:
				pres_result = PresentResult::Success_Suboptimal;
			break;
			case VK_ERROR_OUT_OF_HOST_MEMORY:
				pres_result = PresentResult::Fail_FatalError;
				tz::error("Failed to present Queue because we ran out of host memory (RAM). Please ensure that your system meets the minimum requirements.");
			break;
			case VK_ERROR_OUT_OF_DEVICE_MEMORY:
				pres_result = PresentResult::Fail_FatalError;
				tz::error("Failed to present Queue because we ran out of device memory (VRAM). Please ensure that your system meets the minimum requirements.");
			break;
			case VK_ERROR_DEVICE_LOST:
				pres_result = PresentResult::Fail_FatalError;
				tz::error("Failed to present Queue because device was lost. This is a fatal error.");
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
				tz::error("Failed to present Queue but cannot determine why. Please submit a bug report.");
			break;
		}
		return pres_result;
	}

	Queue::NativeType Queue::native() const
	{
		return this->queue;
	}

	Queue Queue::null()
	{
		return {};
	}

	bool Queue::is_null() const
	{
		return this->queue == VK_NULL_HANDLE;
	}

	bool Queue::operator==(const Queue& rhs) const
	{
		return this->queue == rhs.queue;
	}

	Queue::Queue():
	queue(VK_NULL_HANDLE),
	info()
	{
	}

	void Queue::execute_cpu_side_command_buffer(const CommandBuffer& command_buffer) const
	{
		TZ_PROFZONE("Vulkan Backend - Command Buffer CPU Execute", 0xFFAA0000);
		for(const VulkanCommand::variant& cmd : command_buffer.get_recorded_commands())
		{
			std::visit([](auto&& val)
			{
				using T = std::decay_t<decltype(val)>;
				if constexpr(std::is_same_v<T, VulkanCommand::TransitionImageLayout>)
				{
					TZ_PROFZONE("Vulkan Backend - TransitionImageLayout CPU Execute", 0xFFAA0000);
					// We probably need to change the image's layout CPU-side.
					Image& img = *val.image;
					img.set_layout(val.target_layout);
				}
			}, cmd);
		}
	}
}

#endif // TZ_VULKAN
