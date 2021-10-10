#if TZ_VULKAN
#include "gl/impl/backend/vk/submit.hpp"
#include "core/profiling/zone.hpp"

namespace tz::gl::vk
{
	Submit::Submit(CommandBuffers buffers, SemaphoreRefs wait_semaphores, WaitStages wait_stages, SemaphoreRefs signal_semaphores):
	submit(),
	wait_semaphore_natives(),
	command_buffer_natives(),
	signal_semaphore_natives(),
	wait_stages()
	{
		this->submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		for(const CommandBuffer& buf : buffers)
		{
			this->command_buffer_natives.push_back(buf.native());
			this->command_buffer_references.push_back(&buf);
		}

		for(const Semaphore& wait_sem : wait_semaphores)
		{
			this->wait_semaphore_natives.push_back(wait_sem.native());
		}

		for(WaitStage wait_stage : wait_stages)
		{
			this->wait_stages.push_back(static_cast<VkPipelineStageFlags>(wait_stage));
		}

		for(const Semaphore& signal_sem : signal_semaphores)
		{
			this->signal_semaphore_natives.push_back(signal_sem.native());
		}

		this->update();
	}

	void Submit::operator()(const hardware::Queue& queue, const Fence& fence) const
	{
		TZ_PROFZONE("Backend VK : RenderWork Submit w/ Fence", TZ_PROFCOL_RED);
		vkQueueSubmit(queue.native(), 1, &this->submit, fence.native());
		for(const CommandBuffer* cmd_buf : this->command_buffer_references)
		{
			(*cmd_buf)();
		}
	}

	void Submit::operator()(const hardware::Queue& queue) const
	{
		TZ_PROFZONE("Backend VK : RenderWork Submit", TZ_PROFCOL_RED);
		vkQueueSubmit(queue.native(), 1, &this->submit, VK_NULL_HANDLE);
		for(const CommandBuffer* cmd_buf : this->command_buffer_references)
		{
			(*cmd_buf)();
		}
	}

	void Submit::update()
	{
		this->submit.waitSemaphoreCount = this->wait_semaphore_natives.size();
		this->submit.pWaitSemaphores = this->wait_semaphore_natives.data();
		this->submit.pWaitDstStageMask = this->wait_stages.data();

		this->submit.commandBufferCount = this->command_buffer_natives.size();
		this->submit.pCommandBuffers = this->command_buffer_natives.data();

		this->submit.signalSemaphoreCount = this->signal_semaphore_natives.size();
		this->submit.pSignalSemaphores = this->signal_semaphore_natives.data();
	}

}

#endif // TZ_VULKAN